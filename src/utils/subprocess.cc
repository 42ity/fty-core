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

#include <sys/types.h>
#include <signal.h>
#include <time.h>

namespace utils {

// forward declaration of helper functions
// TODO: move somewhere else
char * const * _mk_argv(Argv vec);
void _free_argv(char * const * argv);
std::size_t _argv_hash(Argv args);
        
SubProcess::SubProcess(Argv cxx_argv) :
    _fork(false),
    _state(SubProcessState::NOT_STARTED),
    _cxx_argv(cxx_argv),
    _return_code(-1),
    _core_dumped(false)
{
    _outpair[0] = -1;
    _outpair[1] = -1;
    _errpair[0] = -1;
    _errpair[1] = -1;
}

SubProcess::~SubProcess() {
    int _saved_errno = errno;
    
    //XXX: copy from cxxutils's Fork::~fork()
    if (isRunning()) {
        wait();
    }

    // close pipes
    ::close(_outpair[0]);
    ::close(_errpair[0]);
    ::close(_outpair[1]);
    ::close(_errpair[1]);

    errno = _saved_errno;
}

bool SubProcess::run() {

    // do nothing if some process has been already started
    if (_outpair[0] != -1) {
        return true;
    }

    if (::pipe(_outpair) == -1) {
        return false;
    }
    if (::pipe(_errpair) == -1) {
        return false;
    }

    _fork.fork();
    if (_fork.child()) {

        //FIXME: error checking!
        ::close(_outpair[0]);
        ::close(_errpair[0]);
        ::dup2(_outpair[1], STDOUT_FILENO);
        ::dup2(_errpair[1], STDERR_FILENO);

        auto argv = _mk_argv(_cxx_argv);
        if (!argv) {
            return false;
        }

        // TODO: error checking and reporting to the parent
        ::execvp(argv[0], argv);

    }
    // we are in parent
    _state = SubProcessState::RUNNING;
    ::close(_outpair[1]);
    ::close(_errpair[1]);
    // set the returnCode
    poll();
    return true;
}

int SubProcess::wait(bool no_hangup)
{
    int status;
    
    //thanks tomas for the fix!
    status=-1;

    int options = no_hangup ? WNOHANG : 0;

    if (! isRunning()) {
        return _return_code;
    }

    ::waitpid(getPid(), &status, options);

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
    return kill(15);
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

    for (auto proc_i = _running.begin(); proc_i != _running.end(); ) {

        auto proc = *proc_i;

        proc->poll();
        if (!proc->isRunning()) {
            _done.push_back(proc);
            _running.erase(proc_i);
            _running_c--;
        } else {
            proc_i++;
        }
    }

    // do nothing if we should not or once we've reached a limit
    if (!schedule_new || (_running_c == _running_limit)) {
        return;
    }

    // 2. start enough new tasks
    auto cnt = std::min((_running_limit - _running_c), _incomming.size());
    for (auto i = 0u; i != cnt; i++) {

        auto args = _incomming[0];
        _incomming.pop_front();

        auto proc = new SubProcess(args);
        proc->run();
        _running.push_front(proc);
        _running_c++;
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

// ### helper functions ###
char * const * _mk_argv(Argv vec) {

    char ** argv = (char **) malloc(sizeof(char*) * (vec.size()+1));
    assert(argv);

    for (auto i=0u; i != vec.size(); i++) {

        auto str = vec[i];
        char* dest = (char*) malloc(sizeof(char) + (str.size() + 1));
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

} //namespace utils
