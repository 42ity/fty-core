/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file   web_utils.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief  some helpers for web
 */
#ifndef SRC_SHARED_WEB_UTILS_H_
#define SRC_SHARED_WEB_UTILS_H_

#include <string>
#include <string>
#include <array>
#include <stdarg.h>

#include <tnt/http.h>

//  ###### THOSE DEFINITONS BELOW ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/* \brief helper WebService Error struct to store all important items*/
typedef struct _wserror {
    const char* key;        ///! short key for compile time dispatch
    int http_code;          ///! http_code is HTTP reply code, use HTTP defines
    int err_code;           ///! sw internal error code
    const char* message;    ///! Message explaining the error, can contain printf like formatting chars
} _WSError;

// size of _errors array, keep this up to date unless code won't build
static constexpr size_t _WSErrorsCOUNT = 8;

// typedef for array of errors
typedef std::array<_WSError, _WSErrorsCOUNT> _WSErrors;

static constexpr const _WSErrors _errors = { {
    {.key = "internal-error",           .http_code = HTTP_INTERNAL_SERVER_ERROR, .err_code = 42, .message = "Internal Server Error: %s" },
    {.key = "not-authorized",           .http_code = HTTP_UNAUTHORIZED,          .err_code = 43, .message = "Not authorized"},
    {.key = "element-not-found",        .http_code = HTTP_NOT_FOUND,             .err_code = 44, .message = "Element %s not found"},
    {.key = "not-allowed-method",       .http_code = HTTP_METHOD_NOT_ALLOWED,    .err_code = 45, .message = "Method %s is not allowed"},
    {.key = "not-implemented-method",   .http_code = HTTP_NOT_IMPLEMENTED,       .err_code = 46, .message = "Method %s is not implemented"},
    {.key = "param-required",           .http_code = HTTP_BAD_REQUEST,           .err_code = 47, .message = "Parameter %s is required" },
    {.key = "param-bad-value",          .http_code = HTTP_BAD_REQUEST,           .err_code = 48, .message = "Parameter %s has wrong value, got %s, expected %s" },
    {.key = "syntax-error",             .http_code = HTTP_BAD_REQUEST,           .err_code = 49, .message = "Request document has invalid syntax" },
    } };

template <size_t N>
constexpr ssize_t
_die_idx(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > N, "_die_idx asked for too big N");
    return (_errors.at(N).key == key || _errors.at(N).message == key) ? N: _die_idx<N-1>(_errors, key);
}

template <>
constexpr ssize_t
_die_idx<0>(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > 0 , "_die_idx asked for too big N");
    return (_errors.at(0).key == key || _errors.at(0).message == key) ? 0: -1;
}

static int
_die_vasprintf(
        char **buf,
        const char* format,
        ...) __attribute__ ((format (printf, 2, 3))) __attribute__ ((__unused__));

static int
_die_vasprintf(
        char **buf,
        const char* format,
        ...)
{
    int r;
    int saved_errno = errno;
    va_list args;

    va_start(args, format);
    r = vasprintf(buf, format, args);
    va_end(args);

    errno = saved_errno;
    return r;
}

//  ###### THOSE DEFINITONS ABOVE ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/*
 * \brief print valid json error and die with proper http code
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * can be used as ``http_die("internal-error", e.what())`` or
 * ``http_die("Internal Server Error: %s", e.what())``
 *
 * Mistakes in key strings are compile time errors, so one can't
 * make the mistake even if he want to.
 *
 */

#define http_die(key, ...) \
    do { \
        constexpr ssize_t key_idx = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
        static_assert(key_idx != -1, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
        char *message; \
        _die_vasprintf(&message, _errors.at(key_idx).message, ##__VA_ARGS__ ); \
        reply.out() << create_error_json(message, _errors.at(key_idx).err_code); \
        free(message); \
        return _errors.at(key_idx).http_code;\
    } \
    while(0)

/*
 * \brief Creates a string in the form "key":"value"
 *        without any {,} (just pair key value). As
 *        value is string it should be in quotes.
 *
 * \param key   - a key
 * \param value - a string value
 *
 * \return "key":"value"
 */
std::string
    json_key_value_s(
        const std::string &key,
        const std::string &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * \param key   - a key
 * \param value - an unsigned value
 *
 * \return "key":value
 */
std::string
   json_key_value_u(
        const std::string &key,
        const uint64_t    &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * \param key   - a key
 * \param value - an integer value
 *
 * \return "key":value
 */
std::string
   json_key_value_i(
        const std::string &key,
        const int64_t     &value);

/*
 * \brief Creates a string in the form "key":value
 *        without any {,} (just pair key value). As
 *        value is not a string it should not be in quotes.
 *
 * Pricisios: 6 numbers after point always (even zeros)
 *
 * \param key   - a key
 * \param value - a double value
 *
 * \return "key":value
 */
std::string
   json_key_value_d(
        const std::string &key,
        const double      &value);

/*
 * \brief Creates a string that containes JSON error message
 *        with only one error.
 *
 *
 *  Returned string has the following structure:
 *
 *  {
 *      errors: [
 *          {
 *              "message": "@msg",
 *              "code": @code
 *          }
 *      ]
 *  }
 *
 * \param msg   - a message that should be displayed to the user
 * \param code  - a code for an error
 *
 * \return JSON string
 */
std::string
    create_error_json(
        const std::string &msg,
        int                code);

#endif // SRC_SHARED_WEB_UTILS_H_
