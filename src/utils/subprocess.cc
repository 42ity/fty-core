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

// forward declaration of helper functions
// TODO: move somewhere else
char * const * _mk_argv(std::vector<std::string> vec);
void _free_argv(char * const * argv);
        
SubProcess::SubProcess(std::vector<std::string> cxx_argv) :
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

void SubProcess::run() {

    // do nothing if some process has been already started
    if (_outpair[0] != -1) {
        return;
    }

    if (::pipe(_outpair) == -1) {
        throw std::runtime_error("Failed to create a pipe for stdout");
    }
    if (::pipe(_errpair) == -1) {
        throw std::runtime_error("Failed to create a for stderr");
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
            throw std::runtime_error("memory allocation failed");
        }
        if (::execvp(argv[0], argv) == -1) {
            _free_argv(argv);
            throw std::runtime_error("execvpe failed");
        }
        //_free_argv(argv); can be omited as successful execvpe means we can't reach this place being replaced by different process

    }
    else {
        _state = SubProcessState::RUNNING;
        ::close(_outpair[1]);
        ::close(_errpair[1]);
        // set the returnCode
        poll();
    }
}

int SubProcess::wait(bool no_hangup)
{
    int status, ret;
    
    //thanks tomas for the fix!
    status=-1;

    int options = no_hangup ? WNOHANG : 0;

    if (! isRunning()) {
        return _return_code;
    }

    ret = ::waitpid(getPid(), &status, options);

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

// ### helper functions ###
char * const * _mk_argv(std::vector<std::string> vec) {

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
