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

/*! \file subprocess.h
    \brief (sub) process C++ API
    \author Michal Vyskocil <michalvyskocil@eaton.com>
 */

#ifndef _SRC_SHARED_SUBPROCESS_H
#define _SRC_SHARED_SUBPROCESS_H

#include <cxxtools/posix/fork.h>

#include <climits>
#include <cstring>
#include <unistd.h>

#include <vector>
#include <deque>
#include <string>
#include <sstream>
#include <map>

/* \brief Helper classes for managing processes
 *
 * class SubProcess:
 * The advantage of this class is easyness of usage, as well as readability as
 * it handles several low-level oddities of POSIX/Linux C-API.
 *
 * Note that SubProcess instance is tied to one process only, so cannot be reused
 * to execute more than one subprocess. This is due "simulate" dynamic nature
 * of a processes. Therefor for a code running unspecified amount of processes, 
 * instances must be heap allocated using new constructor.
 *
 * For that reason copy/move constructor and operator are disallowed.
 *
 * Example:
 * \code
 * SubProcess proc("/bin/true");
 * proc.wait();
 * std::cout "process pid: " << proc.getPid() << std::endl;
 * \endcode
 *
 * class ProcessQue:
 * This maintains a queue of a processes. There are three queues: incomming,
 * running and done. Those are updated only at schedule() call, when finished
 * processes are moved to done and those from the incomming queue are started
 * and moved to running.
 *
 * class ProcCache:
 * Maintains stdout/stderr output for a process in
 * std::ostringstream. It is helper class for
 *
 * class ProcCacheMap:
 * Maintain a cache for set of processes - each is identified by pid
 * (/todo think about const SubProcess *). It stores output of several
 * processes run and poll'ed in parallel.
 */

namespace shared {

//! \brief list of arguments    
typedef std::vector<std::string> Argv;

class SubProcess {
    public:
       
        static const int codeRunning = INT_MIN;
        static const int PIPE_DEFAULT = -1;
        static const int PIPE_DISABLED = -2;

        // \brief construct instance
        //
        // @param argv - C-like string of argument, see execvpe(2) for details
        // @param stdout_pipe - create a pipe for stdout (default yes)
        // @param stderr_pipe - create a pipe for stderr (default yes)
        //
        // \todo does not deal with a command line limit
        explicit SubProcess(Argv cxx_argv, bool stdout_pipe = true, bool stderr_pipe = true);

        // \brief close all pipes, waits on process termination
        //
        // \warning destructor calls wait, so can hand your program in a case
        //          child process never ends. Better to call terminate() manually.
        //
        virtual ~SubProcess();

        // \brief return the commandline
        const Argv& argv() const { return _cxx_argv; }

        // \brief return the commandline as a space delimited string
        std::string argvString() const;
        
        //! \brief return pid of executed command
        pid_t getPid() const { return _fork.getPid(); }
        
        //! \brief get the pipe ends connected to stdout of started program, or -1 if not started
        int getStdout() const { return _outpair[0]; }
        
        //! \brief get the pipe ends connected to stderr of started program, or -1 if not started
        int getStderr() const { return _errpair[0]; }
        
        //! \brief does program run or not
        //
        //  isRunning checks the status code, so wait/poll function calls are necessary
        //  in order to see the real state of a process
        //  \todo - call kill(0) + wait if not running?
        bool isRunning() const { return _state == SubProcessState::RUNNING; }

        //! \brief get the return code, \see wait for meaning
        int getReturnCode() const { return _return_code; }

        //! \brief return core dumped flag
        bool isCoreDumped() const { return _core_dumped; }

        // \brief creates a pipe/pair for stdout/stderr, fork and exec the command. Note this
        // can be started only once, all subsequent calls becames nooop and return true.
        //
        // @return true if exec was successfull, otherwise false and reason is in errno
        //
        // \todo error reporting from a child
        //
        bool run();

        //! \brief wait on program terminate
        //
        //  @param no_hangup if false (default) wait indefinitelly, otherwise return immediatelly
        //  @return positive return value of a process
        //          negative is a number of a signal which terminates process
        //          or codeRunning constant indicating code is still running
        int wait(bool no_hangup=false);

        //! \brief no hanging varint of /see wait
        int poll() {  return wait(true); }

        //! \brief kill the subprocess with defined signal, default SIGTERM/15
        //
        //  @return see kill(2)
        //  \todo - to throw an exception (signal != 0)?
        int kill(int signal=SIGTERM);

        //! \brief terminate the subprocess with SIGKILL/9
        //
        //  This calls wait() to ensure we are not creating zombies
        //
        //  @return \see kill
        int terminate();
    
    protected:

        enum class SubProcessState {
            NOT_STARTED,
            RUNNING,
            FINISHED
        };
        const char* str_state(SubProcessState state);

        cxxtools::posix::Fork _fork;
        SubProcessState _state;
        Argv _cxx_argv;
        int _return_code;
        bool _core_dumped;
        int _outpair[2];
        int _errpair[2];

        // disallow copy and move constructors
        SubProcess(const SubProcess& p) = delete;
        SubProcess& operator=(SubProcess p) = delete;
        SubProcess(const SubProcess&& p) = delete;
        SubProcess& operator=(SubProcess&& p) = delete;

};

class ProcessQue {

    public:

        typedef std::deque<SubProcess*>::const_iterator const_iterator;

        // \brief construct instance
        //
        // @param argv - maximum number of processes to run in parallel
        //
        explicit ProcessQue(std::size_t limit = 4) :
            _running_limit(limit),
            _incomming(),
            _running(),
            _done()
        {
        }

        virtual ~ProcessQue();

        // \brief return const iterator to begining of a list of running processes
        const_iterator cbegin() const;
        // \brief return const iterator to the end of a list of running processes
        const_iterator cend() const;
        // \brief returns if there are processes in done queue
        bool hasDone() const;
        // \brief returns if there are processes in an incomming queue
        bool hasIncomming() const;
        // \brief returns if there are processes in an running queue
        bool hasRunning() const;
        // \brief returns the size of running queue
        std::size_t runningSize() const;
        
        // \brief remove the SubProcess from done queue and return it
        SubProcess* pop_done();
        // \brief add new task to queue
        bool add(Argv &args);
        // \brief schedule new processes
        //
        // @param schedule_new - to start new processes (default)
        void schedule(bool schedule_new=true);
        // \brief terminate all running processes and update the queue using schedule(false)
        void terminateAll();


    protected:
        std::size_t _running_limit;
        std::deque<Argv> _incomming;
        std::deque<SubProcess*> _running;
        std::deque<SubProcess*> _done;

        // disallow copy and move constructors
        ProcessQue(const ProcessQue& p) = delete;
        ProcessQue& operator=(ProcessQue p) = delete;
        ProcessQue(const ProcessQue&& p) = delete;
        ProcessQue& operator=(ProcessQue&& p) = delete;
};

//! caches process's stdout/stderr in std::ostringstream
class ProcCache {
    public:

        // \brief construct instance
        ProcCache():
            _ocache{},
            _ecache{}
        {}

        // \brief copy instance
        ProcCache (const ProcCache &cache) {
            _ocache.str(cache._ocache.str());
            _ocache.clear();
            _ecache.str(cache._ecache.str());
            _ecache.clear();
        }

        // \brief move
        ProcCache& operator=(const ProcCache &cache) {
            _ocache.str(cache._ocache.str());
            _ocache.clear();
            _ecache.str(cache._ecache.str());
            _ecache.clear();
            return *this;
        }

        // \brief push new stuff to cache of stdout
        void pushStdout(const char* str);
        // \brief push new stuff to cache of stdout
        void pushStdout(const std::string& str);

        // \brief push new stuff to cache of stderr
        void pushStderr(const char* str);
        // \brief push new stuff to cache of stderr
        void pushStderr(const std::string& str);

        // \brief pop cached values for stdout/stderr
        std::pair<std::string, std::string> pop();

    protected:

        std::ostringstream _ocache;
        std::ostringstream _ecache;
};

//! map<pid_t, ProcCache> with ProcCache-like API
class ProcCacheMap {

    public:

        // \brief construct instance
        ProcCacheMap():
            _map()
        {};

        // \brief is pid in a map?
        bool hasPid(pid_t pid) const;

        // \brief push new stuff to cache of stdout
        void pushStdout(pid_t pid, const char* str);
        // \brief push new stuff to cache of stdout
        void pushStdout(pid_t pid, const std::string& str);

        // \brief push new stuff to cache of stderr
        void pushStderr(pid_t pid, const char* str);
        // \brief push new stuff to cache of stderr
        void pushStderr(pid_t pid, const std::string& str);

        std::pair<std::string, std::string> pop(pid_t pid);

    protected:
        typedef std::map<pid_t, ProcCache> map_type;
        map_type _map;

        void _push_cstr(pid_t pid, const char* str, bool push_stdout);
        void _push_str(pid_t pid, const std::string& str, bool push_stdout);
};


} //namespace shared

#endif //_SRC_SHARED_SUBPROCESS_H

