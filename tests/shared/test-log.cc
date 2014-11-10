#include <catch.hpp>

#include <cstdio>
#include <unistd.h>

#include <utils.h>
#include <log.h>

// mkstemp returning FILE* instead of int
FILE *mksftemp(char *templ) {
    FILE *ret = NULL;
    int fd = mkstemp(templ);

    if (fd == -1) {
        goto end;
    }

    ret = fdopen(fd, "r+");
    if (!ret) {
        close(fd);
        goto end;
    }

end:
    return ret;
}

TEST_CASE("log-getset-level", "[log][level]") {
    // log_level is initially set to LOG_SYSLOG_NA
    CHECK(log_get_syslog_level() == LOG_SYSLOG_NA);
    CHECK(log_get_stderr_level() == LOG_ERR);

    log_set_syslog_level(LOG_DEBUG);
    CHECK(log_get_syslog_level() == LOG_DEBUG);
    CHECK(log_get_stderr_level() == LOG_ERR);
    
    log_set_stderr_level(LOG_WARNING);
    CHECK(log_get_syslog_level() == LOG_DEBUG);
    CHECK(log_get_stderr_level() == LOG_WARNING);

    log_set_level(LOG_CRIT);
    CHECK(log_get_syslog_level() == LOG_CRIT);
    CHECK(log_get_stderr_level() == LOG_CRIT);
}

TEST_CASE("log-getset-stderr", "[log][stderr]") {
    CHECK(log_get_file() == stderr);

    log_set_file(stdout);
    CHECK(log_get_file() == stdout);
}

TEST_CASE("log-do_log", "[log][do_log]") {

    char temp_name[128];
    sprintf(temp_name, "test-log.XXXXXX");
    FILE *tempf = mksftemp(temp_name);

    //XXX: all get/set modify global state of log.lo
    //thus sanitize manually
    log_set_syslog_level(LOG_NOOP);
    log_set_stderr_level(LOG_ERR);
    log_set_file(tempf);
    CHECK(log_get_stderr_level() == LOG_ERR);
    CHECK(log_get_syslog_level() < LOG_CRIT);

    //XXX: don't use log_XXX macros as the subsequent check
    //will be complicated - TODO: check usage of regexes
    do_log(LOG_CRIT,
            "test-log",
            42,
            "test_do_log",
            "testing C-%s string",
            "formatted");

    int ret = fflush(tempf);
    if (ret != 0) {
        fprintf(stderr, "[ERROR]: fail to sync '%s': %m\n", temp_name);
        exit(1);
    }
    rewind(tempf);

    char buf[1024];
    size_t r = fread((void*) buf, 1, 64, tempf);

    CHECK(r == 64);
    buf[64] = 0;
    CHECK(streq(buf, "[CRITICAL]: test-log:42 (test_do_log) testing C-formatted string"));

    fclose(tempf);
    unlink(temp_name);

}
