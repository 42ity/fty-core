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

#include "subprocess.h"

#include <cassert>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>

#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

namespace shared {

// forward declaration of helper functions
// TODO: move somewhere else
char * const * _mk_argv(const Argv& vec);
void _free_argv(char * const * argv);
std::size_t _argv_hash(Argv args);
        
SubProcess::SubProcess(Argv cxx_argv, int flags) :
    _fork(false),
    _state(SubProcessState::NOT_STARTED),
    _cxx_argv(cxx_argv),
    _return_code(-1),
    _core_dumped(false)
{
    // made more verbose to increase readability of the code
    int stdin_flag = PIPE_DISABLED;
    int stdout_flag = PIPE_DISABLED;
    int stderr_flag = PIPE_DISABLED;

    if ((flags & SubProcess::STDIN_PIPE) != 0) {
        stdin_flag = PIPE_DEFAULT;
    }
    if ((flags & SubProcess::STDOUT_PIPE) != 0) {
        stdout_flag = PIPE_DEFAULT;
    }
    if ((flags & SubProcess::STDERR_PIPE) != 0) {
        stderr_flag = PIPE_DEFAULT;
    }

    _inpair[0]  = stdin_flag,  _inpair[1]  = stdin_flag;
    _outpair[0] = stdout_flag, _outpair[1] = stdout_flag;
    _errpair[0] = stderr_flag, _errpair[1] = stderr_flag;
}

SubProcess::~SubProcess() {
    int _saved_errno = errno;
   
    // Graceful shutdown
    if (isRunning())
        kill(SIGTERM);
    for (int i = 0; i<20 && isRunning(); i++) {
        usleep(100);
    }
    if (isRunning()) {
        terminate();
        wait();
    }

    // close pipes
    ::close(_inpair[0]);
    ::close(_outpair[0]);
    ::close(_errpair[0]);
    ::close(_inpair[1]);
    ::close(_outpair[1]);
    ::close(_errpair[1]);

    errno = _saved_errno;
}

//note: the extra space at the end of the string doesn't really matter
std::string SubProcess::argvString() const
{
    std::string ret;
    for (std::size_t i = 0, l = _cxx_argv.size();
         i < l;
         ++i) {
        ret.append (_cxx_argv.at(i));
        ret.append (" ");
    }
    return ret;
}

bool SubProcess::run() {

    // do nothing if some process has been already started
    if (_state != SubProcessState::NOT_STARTED) {
        return true;
    }

    if (_inpair[0] != PIPE_DISABLED && ::pipe(_inpair) == -1) {
        return false;
    }
    if (_outpair[0] != PIPE_DISABLED && ::pipe(_outpair) == -1) {
        return false;
    }
    if (_errpair[0] != PIPE_DISABLED && ::pipe(_errpair) == -1) {
        return false;
    }

    _fork.fork();
    if (_fork.child()) {

        if (_inpair[0] != PIPE_DISABLED) {
            int o_flags = fcntl(_inpair[0], F_GETFL);
            int n_flags = o_flags & (~O_NONBLOCK);
            fcntl(_inpair[0], F_SETFL, n_flags);
            ::dup2(_inpair[0], STDIN_FILENO);
            ::close(_inpair[1]);
        }
        if (_outpair[0] != PIPE_DISABLED) {
            ::close(_outpair[0]);
            ::dup2(_outpair[1], STDOUT_FILENO);
        }
        if (_errpair[0] != PIPE_DISABLED) {
            ::close(_errpair[0]);
            ::dup2(_errpair[1], STDERR_FILENO);
        }

        auto argv = _mk_argv(_cxx_argv);
        if (!argv) {
            return false;
        }

        int ret = ::execvp(argv[0], argv);
        if (ret == -1)
            exit(errno);

    }
    // we are in parent
    _state = SubProcessState::RUNNING;
    ::close(_inpair[0]);
    ::close(_outpair[1]);
    ::close(_errpair[1]);
    // set the returnCode
    poll();
    return true;
}

int SubProcess::wait(bool no_hangup)
{
    //thanks tomas for the fix!
    int status = -1;

    int options = no_hangup ? WNOHANG : 0;

    if (! isRunning()) {
        return _return_code;
    }

    int ret = ::waitpid(getPid(), &status, options);
    if (no_hangup && ret == 0) {
        // state did not change here
        return _return_code;
    }

    if (WIFEXITED(status)) {
        _state = SubProcessState::FINISHED;
        _return_code = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status)) {
        _state = SubProcessState::FINISHED;
        _return_code = - WTERMSIG(status);

        if (WCOREDUMP(status)) {
            _core_dumped = true;
        }
    }
    // we don't allow wait on SIGSTOP/SIGCONT, so WIFSTOPPED/WIFCONTINUED
    // were omited here

    return _return_code;
}
        
int SubProcess::kill(int signal) {
    return ::kill(getPid(), signal); 
}

int SubProcess::terminate() {
    auto ret = kill(SIGKILL);
    wait();
    return ret;
}

const char* SubProcess::state() const {
    if (_state == SubProcess::SubProcessState::NOT_STARTED) {
        return "not-started";
    }
    else if (_state == SubProcess::SubProcessState::RUNNING) {
        return "running";
    }
    else if (_state == SubProcess::SubProcessState::FINISHED) {
        return "finished";
    }

    return "unimplemented state";
}

ProcessQue::~ProcessQue() {

    for (auto taskp: _running) {
        taskp->wait();
        delete taskp;
    }
    
    for (auto taskp: _done) {
        //avoids zombies, calls ::waitpid at least once
        taskp->poll();
        delete taskp;
    }
}

bool ProcessQue::add(Argv &args) {
    //avoid duplicates in _incomming and _running - use task.hash for that
    _incomming.push_back(args);
    return true;
}

void ProcessQue::schedule(bool schedule_new) {
    //1. check a status of _running
    //TODO: can't this work with for (auto &taskp_i: _running)??

    for (auto proc: _running) {
        proc->poll();
        if (!proc->isRunning()) {
            _done.push_back(proc);
        }
    }

    // http://stackoverflow.com/a/9053941
    _running.erase(
            std::remove_if(_running.begin(), _running.end(),
            [] (SubProcess *proc) -> bool {return !proc->isRunning();}),
            _running.end());

    // do nothing if we should not or once we've reached a limit
    if (!schedule_new || (_running.size() == _running_limit)) {
        return;
    }

    // 2. start enough new tasks
    auto cnt = std::min((_running_limit - _running.size()), _incomming.size());
    for (auto i = 0u; i < cnt; i++) {

        auto args = _incomming[0];
        _incomming.pop_front();

        auto proc = new SubProcess(args, _flags);
        proc->run();
        _running.push_front(proc);
    }
}

std::deque<SubProcess*>::const_iterator
    ProcessQue::cbegin() const {
        return _running.cbegin();
}
std::deque<SubProcess*>::const_iterator
    ProcessQue::cend() const {
        return _running.cend();
}

SubProcess* ProcessQue::pop_done() {
   auto ret = _done[0];
   _done.pop_front();
   return ret;
}

bool ProcessQue::hasDone() const {
    return !_done.empty();
}

bool ProcessQue::hasIncomming() const {
    return !_incomming.empty();
}

bool ProcessQue::hasRunning() const {
    return !_running.empty();
}

std::size_t ProcessQue::runningSize() const {
    return _running.size();
}

void ProcessQue::terminateAll() {
    auto it = cbegin();
    while (it != cend()) {
        SubProcess *proc = *it;
        proc->terminate();
        it++;
        usleep(50);
        proc->poll();
    }
    schedule(false);
}

void ProcCache::pushStdout(const char* str) {
    _ocache << str;
}
void ProcCache::pushStdout(const std::string& str) {
    _ocache << str;
}

void ProcCache::pushStderr(const char* str) {
    _ecache << str;
}
void ProcCache::pushStderr(const std::string& str) {
    _ecache << str;
}

std::pair<std::string, std::string> ProcCache::pop() {
    std::pair<std::string, std::string> ret = std::make_pair(
            _ocache.str(),
            _ecache.str());
    _ocache.str(std::string());
    _ocache.clear();
    _ecache.str(std::string());
    _ecache.clear();
    return ret;
}

bool ProcCacheMap::hasPid(pid_t pid) const {
    return (_map.count(pid) == 1);
}

void ProcCacheMap::pushStdout(pid_t pid, const char* str) {
    _push_cstr(pid, str, true);
}

void ProcCacheMap::pushStdout(pid_t pid, const std::string& str) {
    _push_str(pid, str, true);
}

void ProcCacheMap::pushStderr(pid_t pid, const char* str) {
    _push_cstr(pid, str, false);
}

void ProcCacheMap::pushStderr(pid_t pid, const std::string& str) {
    _push_str(pid, str, false);
}

void ProcCacheMap::_push_cstr(pid_t pid, const char* str, bool push_stdout) {
    if (!hasPid(pid)) {
        _map[pid] = ProcCache{};
    }
    if (push_stdout) {
        _map[pid].pushStdout(str);
    }
    else {
        _map[pid].pushStdout(str);
    }

}

void ProcCacheMap::_push_str(pid_t pid, const std::string& str, bool push_stdout) {
    if (!hasPid(pid)) {
        _map[pid] = ProcCache{};
    }
    if (push_stdout) {
        _map[pid].pushStdout(str);
    }
    else {
        _map[pid].pushStdout(str);
    }

}

std::pair<std::string, std::string> ProcCacheMap::pop(pid_t pid) {
    std::pair<std::string, std::string> ret = _map[pid].pop();
    _map.erase(pid);
    return ret;
}

std::string read_all(int fd) {
    static size_t BUF_SIZE = 4096;
    char buf[BUF_SIZE+1];
    ssize_t r;

    std::stringbuf sbuf;

    while (true) {
        memset(buf, '\0', BUF_SIZE+1);
        r = ::read(fd, buf, BUF_SIZE);
        //TODO what to do if errno != EAGAIN | EWOULDBLOCK
        if (r <= 0) {
            break;
        }
        sbuf.sputn(buf, strlen(buf));
    }
    return sbuf.str();
}

std::string wait_read_all(int fd) {
    static size_t BUF_SIZE = 4096;
    char buf[BUF_SIZE+1];
    ssize_t r;
    int exit = 0;

    int o_flags = fcntl(fd, F_GETFL);
    int n_flags = o_flags | O_NONBLOCK;
    fcntl(fd, F_SETFL, n_flags);

    std::stringbuf sbuf;
    memset(buf, '\0', BUF_SIZE+1);
    errno = 0;
    while (::read(fd, buf, BUF_SIZE) <= 0 &&
           (errno == EAGAIN || errno == EWOULDBLOCK) && exit < 5000) {
        usleep(1000);
        errno = 0;
        exit++;
    }

    sbuf.sputn(buf, strlen(buf));

    exit = 0;
    while (true) {
        memset(buf, '\0', BUF_SIZE+1);
        errno = 0;
        r = ::read(fd, buf, BUF_SIZE);
        if (r <= 0) {
            if(exit > 0 || (errno != EAGAIN && errno != EWOULDBLOCK))
                break;
            usleep(1000);
            exit = 1;
        } else {
            exit = 0;
        }
        sbuf.sputn(buf, strlen(buf));
    }
    fcntl(fd, F_SETFL, o_flags);
    return sbuf.str();
}

int call(const Argv& args) {
    SubProcess p(args);
    p.run();
    return p.wait();
}

int output(const Argv& args, std::string& o, std::string& e) {
    SubProcess p(args, SubProcess::STDOUT_PIPE | SubProcess::STDERR_PIPE);
    p.run();
    int ret = p.wait();

    o.assign(read_all(p.getStdout()));
    e.assign(read_all(p.getStderr()));
    return ret;
}

// ### helper functions ###
char * const * _mk_argv(const Argv& vec) {

    char ** argv = (char **) malloc(sizeof(char*) * (vec.size()+1));
    assert(argv);

    for (auto i=0u; i != vec.size(); i++) {

        auto str = vec[i];
        char* dest = (char*) malloc(sizeof(char) * (str.size() + 1));
        memcpy(dest, str.c_str(), str.size());
        dest[str.size()] = '\0';

        argv[i] = dest;
    }
    argv[vec.size()] = NULL;
    return (char * const*)argv;
}

void _free_argv(char * const * argv) {
    char *foo;
    std::size_t n;

    n = 0;
    while((foo = argv[n]) != NULL) {
        free(foo);
        n++;
    }
    free((void*)argv);
}

std::size_t _argv_hash(Argv args) {


    std::hash<std::string> hash;
    size_t ret = hash("");
    
    for (auto str : args) {
        size_t foo = hash(str);
        ret = ret ^ (foo << 1);
    }
    
    return ret;
}

} //namespace shared

