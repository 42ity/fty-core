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
 * \file   utils_web.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief  some helpers for web
 */
#ifndef SRC_SHARED_WEB_UTILS_H_
#define SRC_SHARED_WEB_UTILS_H_

#include <string>
#include <vector>
#include <utility>
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
static constexpr size_t _WSErrorsCOUNT = 9;

// typedef for array of errors
typedef std::array<_WSError, _WSErrorsCOUNT> _WSErrors;

static constexpr const _WSErrors _errors = { {
    {.key = "internal-error",           .http_code = HTTP_INTERNAL_SERVER_ERROR,    .err_code = 42,     .message = "Internal Server Error. %s" },
    {.key = "not-authorized",           .http_code = HTTP_UNAUTHORIZED,             .err_code = 43,     .message = "You are not authorized. Please use '/oauth2/token?username=<user_name>&password=<password>&grant_type=password' GET request to authorize."},
    {.key = "element-not-found",        .http_code = HTTP_NOT_FOUND,                .err_code = 44,     .message = "Element '%s' not found."},
    {.key = "method-not-allowed",       .http_code = HTTP_METHOD_NOT_ALLOWED,       .err_code = 45,     .message = "Http method '%s' not allowed."},
    {.key = "request-param-required",   .http_code = HTTP_BAD_REQUEST,              .err_code = 46,     .message = "Parameter '%s' is required." },
    {.key = "request-param-bad",        .http_code = HTTP_BAD_REQUEST,              .err_code = 47,     .message = "Parameter '%s' has bad value. Received '%s'. Expected %s" },
    {.key = "bad-request-document",     .http_code = HTTP_BAD_REQUEST,              .err_code = 48,     .message = "Request document has invalid syntax. %s" },
    {.key = "data-conflict",            .http_code = HTTP_CONFLICT,              .err_code = 50, .message = "Element '%s' cannot be processed because of conflict. %s"}
    } };

template <size_t N>
constexpr ssize_t
_die_idx(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > N, "_die_idx asked for too big N");
    return (_errors.at(N).key == key || _errors.at(N).message == key) ? N: _die_idx<N-1>(key);
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
        reply.out() << utils::json::create_error_json(message, _errors.at(key_idx).err_code); \
        free(message); \
        return _errors.at(key_idx).http_code;\
    } \
    while(0)


namespace utils {
namespace json {

/*!
 \brief Escape string for json output
 \return Escaped json on success, "(null_ptr)" string on null argument
*/
std::string escape (const char *string);

/*!
 \brief Convenient wrapper for escape"("const char *string")"
*/
std::string escape (const std::string& before);

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T t) {
    try {
        return escape (std::to_string (t));
    } catch (...) {
        return "";
    }
}

// TODO: doxy
// basically, these are property "jsonifyrs"; you supply any json-ifiable type pair and it creates a valid, properly escaped property (key:value) pair out of it.
// single arg version escapes and quotes were necessary (i.e. except int types...)

template <typename T
        , typename = typename std::enable_if<std::is_convertible<T, std::string>::value>::type>
std::string jsonify (const T& t) {
    try {
        return std::string ("\"").append (escape (t)).append ("\"");
    } catch (...) {
        return "";
    }
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_convertible<T, std::string>::value>::type>
std::string jsonify (const S& key, const T& value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (const S& key, T value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename = typename std::enable_if<std::is_convertible<S, std::string>::value>::type
        , typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T key, const S& value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T key, T value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

std::string
create_error_json (const std::string& message, uint32_t code);

std::string
create_error_json (std::vector <std::pair<uint32_t, std::string>> messages);

} // namespace utils::json
} // namespace utils

#endif // SRC_SHARED_WEB_UTILS_H_
