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

        static const int STDIN_PIPE=0x01;
        static const int STDOUT_PIPE=0x02;
        static const int STDERR_PIPE=0x04;
       
        static const int PIPE_DEFAULT = -1;
        static const int PIPE_DISABLED = -2;

        // \brief construct instance
        //
        // @param argv  - C-like string of argument, see execvpe(2) for details
        // @param flags - controll the creation of stdin/stderr/stdout pipes, default no
        //
        // \todo TODO does not deal with a command line limit
        explicit SubProcess(Argv cxx_argv, int flags=0);

        // \brief gracefully kill/terminate the process and close all pipes
        virtual ~SubProcess();

        // \brief return the commandline
        const Argv& argv() const { return _cxx_argv; }

        // \brief return the commandline as a space delimited string
        std::string argvString() const;
        
        //! \brief return pid of executed command
        pid_t getPid() const { return _fork.getPid(); }
        
        //! \brief get the pipe ends connected to stdin of started program, or -1 if not started
        int getStdin() const { return _inpair[1]; }

        //! \brief get the pipe ends connected to stdout of started program, or -1 if not started
        int getStdout() const { return _outpair[0]; }
        
        //! \brief get the pipe ends connected to stderr of started program, or -1 if not started
        int getStderr() const { return _errpair[0]; }

        //! \brief returns last checked status of the process 
        //
        //  Checks the status field only, so wait/poll function calls are necessary
        //  in order to see the real state of a process
        bool isRunning() const { return _state == SubProcessState::RUNNING; }

        //! \brief get the return code, \see wait for meaning
        int getReturnCode() const { return _return_code; }

        //! \brief return core dumped flag
        bool isCoreDumped() const { return _core_dumped; }

        // \brief creates a pipe/pair for stdout/stderr, fork and exec the command. Note this
        // can be started only once, all subsequent calls becames nooop and return true.
        //
        // @return true if exec was successfull, otherwise false and reason is in errno
        bool run();

        //! \brief wait on program terminate
        //
        //  @param no_hangup if false (default) wait indefinitelly, otherwise return immediatelly
        //  @return positive return value of a process
        //          negative is a number of a signal which terminates process
        int wait(bool no_hangup=false);

        //! \brief no hanging variant of /see wait
        int poll() {  return wait(true); }

        //! \brief kill the subprocess with defined signal, default SIGTERM/15
        //
        //  @param signal - signal, defaul is SIGTERM
        //
        //  @return see kill(2)
        int kill(int signal=SIGTERM);

        //! \brief terminate the subprocess with SIGKILL/9
        //
        //  This calls wait() to ensure we are not creating zombies
        //
        //  @return \see kill
        int terminate();

        const char* state() const;
    
    protected:

        enum class SubProcessState {
            NOT_STARTED,
            RUNNING,
            FINISHED
        };

        cxxtools::posix::Fork _fork;
        SubProcessState _state;
        Argv _cxx_argv;
        int _return_code;
        bool _core_dumped;
        int _inpair[2];
        int _outpair[2];
        int _errpair[2];

        // disallow copy and move constructors
        SubProcess(const SubProcess& p) = delete;
        SubProcess& operator=(SubProcess p) = delete;
        SubProcess(const SubProcess&& p) = delete;
        SubProcess& operator=(SubProcess&& p) = delete;

};


// TODO legacy code
class ProcessQue {

    public:

        typedef std::deque<SubProcess*>::const_iterator const_iterator;

        // \brief construct instance
        //
        // @param argv - maximum number of processes to run in parallel
        // @param flags - flags passed to the SubProcess constructor
        //
        explicit ProcessQue(std::size_t limit = 4, int flags = 0) :
            _running_limit(limit),
            _flags{flags},
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
        int _flags;
        std::deque<Argv> _incomming;
        std::deque<SubProcess*> _running;
        std::deque<SubProcess*> _done;

        // disallow copy and move constructors
        ProcessQue(const ProcessQue& p) = delete;
        ProcessQue& operator=(ProcessQue p) = delete;
        ProcessQue(const ProcessQue&& p) = delete;
        ProcessQue& operator=(ProcessQue&& p) = delete;
};


// TODO and this one also is a legacy code
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

// TODO seems this on is a legacy code
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

// \brief read all things from file descriptor
//
// try to read as much as possible from file descriptor and return it as std::string
std::string read_all(int fd);

// \brief read all things from file descriptor while compensating for dealys
//
// Try to read as much as possible from file descriptor and return it as
// std::string. But waits for the first string to appear (5s max) and reads
// till the input stops for more than 1ms
std::string wait_read_all(int fd);

// \brief Run command with arguments.  Wait for complete and return the return value.
//
// @return see \SubProcess.wait
int call(const Argv& args);

// \brief Run command with arguments and return its output as a string.
//
// @param args list of command line arguments
// @param o reference to variable will contain stdout
// @param e reference to variable will contain stderr
// @return see \SubProcess.wait for meaning
//
// \warning use only for commands producing less than default pipe capacity (65536 on Linux).
//          Otherwise this call would be blocked indefinitelly.
int output(const Argv& args, std::string& o, std::string& e);

// \brief Run command with arguments and input on stdin and return its output as a string.
//
// @param args list of command line arguments
// @param o reference to variable will contain stdout
// @param e reference to variable will contain stderr
// @param i const reference to variable will contain stdin
// @return see \SubProcess.wait for meaning
//
// \warning use only for commands producing less than default pipe capacity (65536 on Linux).
//          Otherwise this call would be blocked indefinitelly.
int
output(
    const Argv& args,
    std::string& o,
    std::string& e,
    const std::string& i);

} //namespace shared

#endif //_SRC_SHARED_SUBPROCESS_H

