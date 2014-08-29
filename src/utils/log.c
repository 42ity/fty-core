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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "log.h"

#define LOG_NOOP LOG_DEBUG + 42

static int log_syslog_level = LOG_ERR;
static int log_stderr_level = LOG_ERR;
static int log_facility = LOG_DAEMON;

extern int errno;

#define ASSERT_LEVEL \
    assert(level == LOG_DEBUG   || \
           level == LOG_INFO    || \
           level == LOG_WARNING || \
           level == LOG_ERR     || \
           level == LOG_CRIT)

void log_set_level(int level) {

    ASSERT_LEVEL;

    log_set_syslog_level(level);
    log_set_stderr_level(level);
}

void log_set_syslog_level(int level) {

    ASSERT_LEVEL;

    LOG_UPTO(level);
    log_syslog_level = level;
}

void log_set_stderr_level(int level) {

    ASSERT_LEVEL;

    log_stderr_level = level;
}

int log_get_syslog_level() {
    return log_syslog_level;
}

int log_get_stderr_level() {
    return log_stderr_level;
}

void log_open() {
    openlog(NULL, LOG_PID, log_facility);
    LOG_UPTO(log_get_syslog_level());
}

void log_close() {
    closelog();
}

static int do_logv(
        int level,
        const char* file,
        int line,
        const char* func,
        const char* format,
        va_list args) {

    char *prefix;
    char *header;
    char *fmt;
    char *buffer;

    int r;

    if (level > log_get_syslog_level() && level > log_get_stderr_level()) {
        //no-op if logging disabled
        return 0;
    }

    switch (level) {
        case LOG_DEBUG:
            prefix = "DEBUG"; break;
        case LOG_INFO:
            prefix = "INFO"; break;
        case LOG_WARNING:
            prefix = "WARNING"; break;
        case LOG_ERR:
            prefix = "ERROR"; break;
        case LOG_CRIT:
            prefix = "CRITICAL"; break;
        default:
            fprintf(stderr, "[ERROR]: %s:%d (%s) invalid log level %d\n", __FILE__, __LINE__, __func__, level);
            return -1;
    };

    r = asprintf(&header, "[%s]: %s:%d (%s)", prefix, file, line, func);
    if (r == -1) {
        fprintf(stderr, "[ERROR]: %s:%d (%s) can't allocate enough memory for header string: %m\n", __FILE__, __LINE__, __func__);
        return r;
    }

    r = asprintf(&fmt, "%s %s", header, format);
    free(header);   // we don't need it in any case
    if (r == -1) {
        fprintf(stderr, "[ERROR]: %s:%d (%s) can't allocate enough memory for format string: %m\n", __FILE__, __LINE__, __func__);
        return r;
    }
    
    r = vasprintf(&buffer, fmt, args);
    free(fmt);   // we don't need it in any case
    if (r == -1) {
        fprintf(stderr, "[ERROR]: %s:%d (%s) can't allocate enough memory for message string: %m\n", __FILE__, __LINE__, __func__);
        return r;
    }

    if (log_get_stderr_level() <= log_stderr_level) {
        fputs(buffer, stderr);
    }
    if (log_get_syslog_level() <= log_syslog_level) {
        syslog(level, buffer);
    }

    free(buffer);

    return 0;

}

int do_log(
        int level,
        const char* file,
        int line,
        const char* func,
        const char* format,
        ...) {

    int r;
    int saved_errno = errno;
    va_list args;
    
    va_start(args, format);
    r = do_logv(level, file, line, func, format, args);
    va_end(args);
    
    errno = saved_errno;
    return r;
}
