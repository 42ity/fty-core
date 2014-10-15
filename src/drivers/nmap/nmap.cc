/*
Copyright (C) 2014 Eaton
 
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*! \file nmap.cc
    \brief Nmap driver - react on incomming queue and parse results
    \author Michal Vyskocil <michalvyskocil@eaton.com>
 */

#include <set>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>

#include <unistd.h>

#include <czmq.h>

#include "subprocess.h"
#include "nmap_msg.h"
#include "nmap-driver.h"
#include "nmap-parse.h"
#include "log.h"

typedef utils::ProcessQue ProcessQue;
typedef utils::SubProcess SubProcess;
typedef utils::Argv Argv;
typedef utils::ProcCacheMap ProcCacheMap;

// TODO move to defs.h
static const char* DRIVER_NMAP_SOCK = "ipc://@/bios/driver/nmap";

static ProcessQue _que{4};
static ProcCacheMap _pcmap{};
static std::set<int> _fd_set;

static std::map<NmapMethod, Argv> _map = {
    {NmapMethod::DefaultListScan, {NMAP_BIN, "-oX", "-", "-sL"}},
    {NmapMethod::DefaultDeviceScan, {SUDO_BIN, NMAP_BIN, "-oX", "-", "-R", "-sT", "-sU"}}
};

static std::string read_all(int fd) {
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

static int fd_handler (zloop_t *loop, zmq_pollitem_t *item, void *arg);

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

static int timer_handler(zloop_t *loop, int timer_id, void *arg) {

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

        if (proc->getReturnCode() != 0) {
            //\todo how to report scan fail?
        }
        else {
            //\todo make it nicer! There needs to be code mapping argv to NmapMethod
            auto argv = proc->argv();
            if (std::count(argv.begin(), argv.end(), "-sL")) {
                parse_list_scan(p.first.data());
            }
            else {
                parse_device_scan(p.first.data());
            }
        }

        delete proc;
    }

    return 0;
}

static int fd_handler (zloop_t *loop, zmq_pollitem_t *item, void *arg) {
    const SubProcess *proc = static_cast<const SubProcess*>(arg);

    if (proc->getStdout() == item->fd) {
        _pcmap.pushStdout(proc->getPid(), read_all(proc->getStdout()));
    } else {
        _pcmap.pushStderr(proc->getPid(), read_all(proc->getStderr()));
    }
    return 0;
}

static int command_handler (zloop_t *loop, zsock_t *reader, void *_arg) {


    //TODO: can't have multiple args!
    //zstr_recvx(reader, &delim, &command, &arg, NULL);
    nmap_msg_t *msg = nmap_msg_recv (reader);
    // TODO log the message
    
    const char *command = nmap_msg_type (msg);
    const char *arg = (const char*) zlist_head (nmap_msg_args (msg));
    log_info ("\t%s\t%s\n", command, arg);
    

    enum NmapMethod meth;
    if (streq(command, "defaultlistscan")) {
        meth = NmapMethod::DefaultListScan;
    } else if (streq(command, "defaultdevicescan")) {
        meth = NmapMethod::DefaultDeviceScan;
    }
    else {
        nmap_msg_destroy (&msg);
//        free(command);
//        free(arg);
        return 0;
    }

    Argv args = _map[meth];
    args.push_back(arg);

    _que.add(args);

    nmap_msg_destroy (&msg);
//    free(command);
//    free(arg);
    return 0;
}

void nmap_actor (zsock_t *pipe, void *args) {

    log_info ("%s", "nmap_actor() start\n");

    zsock_t *cmdfd = zsock_new_router (DRIVER_NMAP_SOCK);
    assert (cmdfd);
    zpoller_t *poller = zpoller_new (cmdfd, pipe, NULL);
    assert (poller);
    zsock_signal (pipe, 0);

    zloop_t *loop = zloop_new();
    assert(loop);
    int rv = zloop_timer (loop, 2000, 10, timer_handler, NULL);
    assert (rv != -1);
    
    rv = zloop_reader(loop, cmdfd, command_handler, NULL);
    rv = zloop_start(loop);
    assert (rv == 0);

    zloop_destroy(&loop);        
    zpoller_destroy (&poller);
    zsock_destroy (&cmdfd);

    log_info ("%s", "nmap_actor() end\n");
}

int main() {

    log_open();
    log_set_level(LOG_DEBUG);

    zactor_t *nmap = zactor_new (nmap_actor, NULL);
    assert(nmap);
    
    // TODO connect here is ok, the other connects (inside actors) should be binds
    zsock_t *dealer = zsock_new_dealer (DRIVER_NMAP_SOCK);
    assert (dealer);

    nmap_msg_t * msg = nmap_msg_new (NMAP_MSG_SCAN_COMMAND);
    assert (msg);
    nmap_msg_set_type (msg, "%s", "defaultlistscan", NULL);

    zlist_t *zl = zlist_new ();
    char * hv = "10.130.38.200";
    zlist_append (zl, hv);
    nmap_msg_set_args (msg, &zl);
    zlist_destroy (&zl);
//    zstr_sendx(dealer, "defaultlistscan", "10.130.38.200", NULL);
    nmap_msg_send (&msg, dealer);
    
    // TODO - same thing with uninterupability;
    // until next commit, please don't bash me for this
    while (1) {
        zclock_sleep (1000);
    }

    zactor_destroy(&nmap);
    log_info ("%s", "destroying nmap_actor\n");
    log_close ();
    return 0;
}
