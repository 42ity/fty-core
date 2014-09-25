#include "catch.hpp"    //include catch as a first line
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

#include "subprocess.h"

using namespace utils;

TEST_CASE("subprocess-wait-true", "[subprocess][wait]") {

    std::vector<std::string> argv{"/bin/true"};
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    ret = proc.wait();
    CHECK(ret == 0);
}

TEST_CASE("subprocess-wait-false", "[subprocess][wait]") {

    std::vector<std::string> argv{"/bin/false"};
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    ret = proc.wait();
    CHECK(ret == 1);
}

TEST_CASE("subprocess-wait-sleep", "[subprocess][wait]") {

    std::vector<std::string> argv{"/bin/sleep", "3"};
    int ret;
    bool bret;
    time_t start, stop;

    SubProcess proc(argv);
    start = time(NULL);
    REQUIRE(start != -1);
    bret = proc.run();
    CHECK(bret);
    ret = proc.wait();
    stop = time(NULL);
    REQUIRE(stop != -1);
    CHECK(ret == 0);
    CHECK((stop - start) > 2);
}

TEST_CASE("subprocess-read-stderr", "[subprocess][fd]") {
    std::vector<std::string> argv{"/usr/bin/printf"};
    char buf[1023];
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    ret = proc.wait();
    
    //something on stderr
    memset((void*) buf, '\0', 1023);
    read(proc.getStderr(), (void*) buf, 1023);
    CHECK(strlen(buf) > 42);
    
    //nothing on stdout
    memset((void*) buf, '\0', 1023);
    read(proc.getStdout(), (void*) buf, 1023);
    CHECK(strlen(buf) == 0);

    CHECK(ret == 1);
}

TEST_CASE("subprocess-read-stdout", "[subprocess][fd]") {
    std::vector<std::string> argv{"/usr/bin/printf", "the-test\n"};
    char buf[1023];
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    ret = proc.wait();
    
    //nothing on stderr
    memset((void*) buf, '\0', 1023);
    read(proc.getStderr(), (void*) buf, 1023);
    CHECK(strlen(buf) == 0);
    
    //something on stdout
    memset((void*) buf, '\0', 1023);
    read(proc.getStdout(), (void*) buf, 1023);
    CHECK(strlen(buf) == 9);

    CHECK(ret == 0);
}

TEST_CASE("subprocess-poll-sleep", "[subprocess][poll]") {
    std::vector<std::string> argv{"/bin/sleep", "3"};
    int ret, i;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    ret = proc.getReturnCode();

    for (i = 0; i != 6; i++) {
        ret = proc.poll();
        if (! proc.isRunning()) {
            break;
        }
        sleep(1);
    }
    CHECK(i != 0);
    CHECK(!proc.isRunning());
    CHECK(ret == 0);
}

TEST_CASE("subprocess-kill", "[subprocess][kill]") {
    std::vector<std::string> argv{"/bin/sleep", "300"};
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    usleep(50);

    ret = proc.kill();
    CHECK(ret == 0);
    usleep(50);
    // note that getReturnCode does not report anything unless poll is called
    proc.poll();
    ret = proc.getReturnCode();

    CHECK(!proc.isRunning());
    CHECK(ret == -9);
}

TEST_CASE("subprocess-terminate", "[subprocess][kill]") {
    std::vector<std::string> argv{"/bin/sleep", "300"};
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);
    usleep(50);

    ret = proc.terminate();
    CHECK(ret == 0);
    usleep(50);
    proc.poll();
    // note that getReturnCode does not report anything unless poll is called
    ret = proc.getReturnCode();

    CHECK(!proc.isRunning());
    CHECK(ret == -15);
}

TEST_CASE("subprocess-destructor", "[subprocess][wait]") {
    std::vector<std::string> argv{"/bin/sleep", "2"};
    time_t start, stop;
    bool bret;

    start = time(NULL);
    REQUIRE(start != -1);
    {
        SubProcess proc(argv);
        bret = proc.run();
        CHECK(bret);
    } // destructor called here!
    stop = time(NULL);
    REQUIRE(stop != -1);
    CHECK((stop - start) > 1);
}

TEST_CASE("subprocess-external-kill", "[subprocess][wait]") {
    std::vector<std::string> argv{"/bin/sleep", "200"};
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    CHECK(bret);

    kill(proc.getPid(), SIGTERM);
    usleep(50);
    proc.poll();

    CHECK(!proc.isRunning());
    CHECK(proc.getReturnCode() == -15);

}

TEST_CASE("subprocess-run-fail", "[subprocess][run]") {
    std::vector<std::string> argv{"/n/o/b/i/n/a/r/y",};
    int ret;
    bool bret;

    SubProcess proc(argv);
    bret = proc.run();
    //XXX: bret reports only serious errors
    CHECK(bret);
    ret = proc.poll();
    CHECK(ret == -1);
    //XXX: we need to call wait to get the right status - it's weird, but you has to explicitly synch with external reurces every time
    proc.wait();
    CHECK(!proc.isRunning());
}

TEST_CASE("subprocess-proccache", "[subprocess][proccache]") {
    ProcCache c{};

    c.pushStdout("stdout");
    c.pushStderr("s");
    c.pushStderr("td");
    c.pushStderr("err");

    std::pair<std::string, std::string> r = c.pop();
    CHECK(r.first == "stdout");
    CHECK(r.second == "stderr");
    
    //pop means to clear everything - so next attempt is an empty string(s)
    std::pair<std::string, std::string> r2 = c.pop();
    CHECK(r2.first == "");
    CHECK(r2.second == "");

    // test the copy contructor
    c.pushStdout("copy");
    ProcCache c2 = c;
    c.pushStdout(" this");

    r = c.pop();
    r2 = c2.pop();
    CHECK(r.first == "copy this");
    CHECK(r.second == "");
    CHECK(r2.first == "copy");
    CHECK(r2.second == "");

}

TEST_CASE("subprocess-proccachemap", "[subprocess][proccachemap]") {
    ProcCacheMap c{};
    pid_t pid1, pid2;

    pid1 = 1;
    pid2 = 42;

    REQUIRE(!c.hasPid(pid1));
    REQUIRE(!c.hasPid(pid2));

    c.pushStdout(pid1, "stdout");

    CHECK(c.hasPid(pid1));
    CHECK(!c.hasPid(pid2));

    //XXX: I've assumed [] would raise an exception ...
    std::pair<std::string, std::string> r = c.pop(pid2);
    CHECK(r.first == "");
    CHECK(r.second == "");

    CHECK(c.hasPid(pid1));
    CHECK(!c.hasPid(pid2));

    r = c.pop(pid1);
    CHECK(!c.hasPid(pid1));
    CHECK(!c.hasPid(pid2));
    CHECK(r.first == "stdout");
    CHECK(r.second == "");

    r = c.pop(pid1);
    CHECK(!c.hasPid(pid1));
    CHECK(!c.hasPid(pid2));
    CHECK(r.first == "");
    CHECK(r.second == "");

}

TEST_CASE("subprocess-que", "[subprocess][processque]") {
    Argv args{"/bin/cat"};
    ProcessQue q{1};

    CHECK(!q.hasDone());
    CHECK(!q.hasIncomming());
    CHECK(!q.hasRunning());
    CHECK(q.runningSize() == 0);
    
    q.add(args);
    CHECK(!q.hasDone());
    CHECK(q.hasIncomming());
    CHECK(!q.hasRunning());
    CHECK(q.runningSize() == 0);
    
    q.add(args);
    CHECK(!q.hasDone());
    CHECK(q.hasIncomming());
    CHECK(!q.hasRunning());
    CHECK(q.runningSize() == 0);

    fprintf(stderr, "first schedule()\n");
    q.schedule();
    CHECK(!q.hasDone());
    CHECK(q.hasIncomming());
    CHECK(q.hasRunning());
    CHECK(q.runningSize() == 1);
    
    fprintf(stderr, "second schedule()\n");
    //second schedule does not have any impact on runningSize due limit == 1
    q.schedule();
    CHECK(!q.hasDone());
    CHECK(q.hasIncomming());
    CHECK(q.hasRunning());
    CHECK(q.runningSize() == 1);

    //test the iterators + schedule(false) - terminateAll does not
    //start any new jobs
    q.terminateAll();
    //XXX: THE PART BELOW IS EXTREMLY UNRELIABLE - THE BEST
    //I WAS ABLE TO GET WITH A LOT OF USLEEP INSIDE WAS
    //AROUND 75% SUCCESS RATE - SKIPPING FOR NOW
    /*
    for (int i = 0; i != 6; i++) {
        if (q.hasDone()) {
            break;
        }
        usleep(i*100);
    }
    CHECK(q.hasDone());
    CHECK(q.hasIncomming());
    CHECK(!q.hasRunning());
    CHECK(q.runningSize() == 0);
 
    q.schedule();
    CHECK(q.hasDone());
    CHECK(!q.hasIncomming());
    CHECK(q.hasRunning());
    CHECK(q.runningSize() == 1);

    q.terminateAll();
    */

}

