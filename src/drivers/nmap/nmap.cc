
#include <set>
#include <iostream>
#include <sstream>
#include <cassert>

#include <unistd.h>

#include <czmq.h>

#include "subprocess.h"

const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";

typedef utils::ProcessQue ProcessQue;
typedef utils::SubProcess SubProcess;
typedef utils::Argv Argv;
typedef utils::ProcCacheMap ProcCacheMap;

static ProcessQue _que{4};
static ProcCacheMap _pcmap{};
static std::set<int> _fd_set;

enum class NmapMethod {
    DefaultListScan,
    DefaultDeviceScan
};

#define NMAP_BIN "/usr/bin/nmap"
#define SUDO_BIN "/ust/bin/sudo"

static std::map<NmapMethod, Argv> _map = {
    {NmapMethod::DefaultListScan, {NMAP_BIN, "-oX", "-", "-sL"}},
    {NmapMethod::DefaultDeviceScan, {SUDO_BIN, NMAP_BIN, "-oX", "-", "-R", "-sT", "-sU"}}
};

std::string read_all(int fd) {
    static size_t BUF_SIZE = 4096;
    char buf[BUF_SIZE];
    ssize_t r;

    std::stringbuf sbuf;

    while (true) {
        memset(buf, '\0', BUF_SIZE);
        r = ::read(fd, buf, BUF_SIZE);
        //TODO if errno != EAGAIN | EWOULDBLOCK, unregister fd?
        if (r <= 0) {
            break;
        }
        sbuf.sputn(buf, strlen(buf));
    }
    return sbuf.str();
}


int fd_handler (zloop_t *loop, zmq_pollitem_t *item, void *arg);

static int xzloop_registerfd(zloop_t *loop, int fd, SubProcess* proc) {
    if (_fd_set.count(fd) == 1) {
        return 0;
    }
    _fd_set.insert(fd);
    zmq_pollitem_t it = {NULL, fd, ZMQ_POLLIN, 0};
    return zloop_poller (loop, &it, fd_handler, static_cast<void*>(proc));
}

static void xzloop_unregisterfd(zloop_t *loop, int fd) {
    if (_fd_set.count(fd) == 0) {
        return;
    }
    _fd_set.erase(fd);
    zmq_pollitem_t it = {NULL, fd, ZMQ_POLLOUT, 0};
    return zloop_poller_end (loop, &it);
}

int timer_handler(zloop_t *loop, int timer_id, void *arg) {

    _que.schedule();
    for (auto proc_it = _que.cbegin(); proc_it != _que.cend(); proc_it++) {
        auto proc = *proc_it;
        if (!proc) {
            continue;
        }
        xzloop_registerfd(loop, proc->getStdout(), proc);
        xzloop_registerfd(loop, proc->getStderr(), proc);
    }

    while (_que.hasDone()) {
        SubProcess *proc = _que.pop_done();
        xzloop_unregisterfd(loop, proc->getStdout());
        xzloop_unregisterfd(loop, proc->getStderr());

        _pcmap.pushStdout(proc->getPid(), read_all(proc->getStdout()));
        _pcmap.pushStderr(proc->getPid(), read_all(proc->getStderr()));

        //TBD: sending code!
        std::pair<std::string, std::string> p = _pcmap.pop(proc->getPid());
        printf("%s", p.first.data());
        printf("%s", p.second.data());
        delete proc;
    }

    return 0;
}

int fd_handler (zloop_t *loop, zmq_pollitem_t *item, void *arg) {
    const SubProcess *proc = static_cast<const SubProcess*>(arg);

    if (proc->getStdout() == item->fd) {
        _pcmap.pushStdout(proc->getPid(), read_all(proc->getStdout()));
    } else {
        _pcmap.pushStderr(proc->getPid(), read_all(proc->getStderr()));
    }
    return 0;
}

int cmd_streamer(zloop_t *loop, int timer_id, void *arg) {

    zsock_t *fd = zsock_new_dealer(DRIVER_NMAP_SOCK);
    assert(fd);

    zclock_sleep(200); //XXX: wait between dealer socket new and real estabilish ...
    zstr_sendx(fd, "defaultlistscan", "10.130.38.0/24", NULL);
    zclock_sleep(150);

    zsock_destroy(&fd);
}

int command_handler (zloop_t *loop, zsock_t *reader, void *_arg) {

    char* delim;
    char* command;
    char* arg;

    //TODO: can't have multiple args!
    zstr_recvx(reader, &delim, &command, &arg, NULL);
    printf("got '%s', '%s'\n", command, arg);

    enum class NmapMethod meth;
    if (streq(command, "defaultlistscan")) {
        meth = NmapMethod::DefaultListScan;
    } else if (streq(command, "defaultdevicescan")) {
        meth = NmapMethod::DefaultDeviceScan;
    }
    else {
        free(delim);
        free(command);
        free(arg);
        return 0;
    }

    Argv args = _map[meth];
    //todo strdup?
    args.push_back(arg);

    _que.add(args);
    printf("_que.runningSize()-> %ld\n", _que.runningSize());

    free(delim);
    free(command);
    free(arg);
    return 0;
}

int main() {

    int r;

    zsock_t *cmdfd = zsock_new_router(DRIVER_NMAP_SOCK);

    zloop_t *loop = zloop_new();
    assert(loop);

    r = zloop_timer(loop, 2000, 10, timer_handler, NULL);
    assert(r == 0);
    
    r = zloop_timer(loop, 1000, 5, cmd_streamer, NULL);
    assert(r == 0);

    r = zloop_reader(loop, cmdfd, command_handler, NULL);

    r = zloop_start(loop);
    assert(r == 0);

    zloop_destroy(&loop);
    zsock_destroy(&cmdfd);
    return 0;
}
