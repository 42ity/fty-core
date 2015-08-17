/*
Copyright (C) 2014 Eaton
 
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file log.h
    \brief logging API
    \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 
Example:
 
   #include "log.h"

   int main() {

       log_open();

       log_debug("debug level is invisible");
        
       log_set_level(LOG_DEBUG);
        
       log_debug("%s", "debug level is visible");

       log_critical("%s", "critical level");
       log_error("%s", "error level");
       log_warning("%s", "warning level");
       log_info("%s", "info level");
       log_debug("%s", "debug level");

       log_close();
    }
 */

#ifndef _SHARED_SRC_LOG_H
#define _SHARED_SRC_LOG_H

// Trick to avoid conflict with CXXTOOLS logger, currently the BIOS code
// prefers OUR logger macros
#if defined(LOG_CXXTOOLS_H) || defined(CXXTOOLS_LOG_CXXTOOLS_H)
# undef log_error
# undef log_debug
# undef log_info
# undef log_fatal
# undef log_warn
#else
# define LOG_CXXTOOLS_H
# define CXXTOOLS_LOG_CXXTOOLS_H
#endif

#include <stdio.h>
#include <syslog.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_NOOP LOG_EMERG -1
#define LOG_SYSLOG_NA LOG_NOOP -1

/*! \brief open log
 *
 * This function does open a connection to syslog and set the level according value from log_get_syslog_level(). You can omit it if you don't want to log to syslog (for example for cli).
 * */
void log_open();
/*! \brief close log */
void log_close();

/*! \brief set the maximum log level
 *
 * the default value is LOG_ERR, possible values (in order of decreasing importance):
 *  LOG_CRIT    Unrecoverable errors like process managers dies, no MQ, ...
 *  LOG_ERR     Error cases system can recover from (some process segfaults, ...)
 *  LOG_WARNING Cases where some optional functionality can't be used
 *  LOG_INFO    Do we need it?
 *  LOG_DEBUG   Tracing purposes, a lot of details!
 *
 *  Setting LOG_DEBUG means LOG_DEBUG + levels above are printed
 * */
void log_set_level(int level);

/*! \brief set the maximum syslog level */
void log_set_syslog_level(int level);

/*! \brief get the maximum syslog level */
int log_get_syslog_level();

/*! \brief set the maximum stderr level */
void log_set_stderr_level(int level);

/*! \brief get the maximum stderr level */
int log_get_stderr_level();

/*! \brief get the stderr FILE* */
FILE *log_get_file();

/*! \brief set the stderr FILE* */
void log_set_file(FILE* file);

/*! \brief do logging
    An internal logging function, use specific log_error, log_debug  macros!
    \param level - level for message, see \ref log_get_level for legal values
    \param file - name of file issued print, usually content of __FILE__ macro
    \param line - number of line, usually content of __LINE__ macro
    \param func - name of function issued log, usually content of __func__ macro
    \param format - printf-like format string
 */
int do_log(
        int level,
        const char* file,
        int line,
        const char* func,
        const char* format,
        ...) __attribute__ ((format (printf, 5, 6))); 

#define log_macro(level, ...) \
    do { \
        do_log((level), __FILE__, __LINE__, __func__, __VA_ARGS__); \
    } while(0)

/*! \def log_debug(format, ...)
    Prints message with LOG_DEBUG level */
#define log_debug(...) \
        log_macro(LOG_DEBUG, __VA_ARGS__)

/*! \def log_info(format, ...)
    Prints message with LOG_INFO level */
#define log_info(...) \
        log_macro(LOG_INFO, __VA_ARGS__)

/*! \def log_warning(format, ...)
    Prints message with LOG_WARNING level */
#define log_warning(...) \
        log_macro(LOG_WARNING, __VA_ARGS__)

/*! \def log_error(format, ...)
    Prints message with LOG_ERR level */
#define log_error(...) \
        log_macro(LOG_ERR, __VA_ARGS__)

/*! \def log_critical(format, ...)
    Prints message with LOG_CRIT level */
#define log_critical(...) \
        log_macro(LOG_CRIT, __VA_ARGS__)

#define LOG_START \
    log_info("start")

#define LOG_END \
    log_info("end::normal")

#define LOG_END_ABNORMAL(exp) \
    log_warning("end::abnormal with %s", (exp).what())

#ifdef __cplusplus
}
#endif

#endif

