#include "catch.hpp"    //include catch as a first line
#include <cstring>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <signal.h>

#include "subprocess.h"

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
