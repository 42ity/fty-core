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
#include <list>
#include <utility>
#include <array>
#include <stdarg.h>
#include <cmath>
#include <climits>

#include <tnt/http.h>

//  ###### THOSE DEFINITONS BELOW ARE PRIVATE TO http_die AND SHALL NOT BE ACCESSED DIRECTLY

/* \brief helper WebService Error struct to store all important items*/
typedef struct _wserror {
    const char* key;        ///! short key for compile time dispatch
    int http_code;          ///! http_code is HTTP reply code, use HTTP defines
    int err_code;           ///! sw internal error code
    const char* message;    ///! Message explaining the error, can contain printf like formatting chars
} _WSError;

// size of _errors array, keep this up to date otherwise code won't build
static constexpr size_t _WSErrorsCOUNT = 14;

// typedef for array of errors
typedef std::array<_WSError, _WSErrorsCOUNT> _WSErrors;

// WARNING!!! - don't use anything else than %s as format parameter for .message
//
// TL;DR;
// The .messages are supposed to be called with FEWER formatting arguments than defined.
// To avoid issues with going to unallocated memory, the _die_asprintf is called with
// **5** additional empty strings, which fill sefgaults for other formatting specifiers.

#define HTTP_TEAPOT 418 //see RFC2324
static constexpr const _WSErrors _errors = { {
    {.key = "undefined",                .http_code = HTTP_TEAPOT,                   .err_code = INT_MIN, .message = "I'm a teapot!" },
    {.key = "internal-error",           .http_code = HTTP_INTERNAL_SERVER_ERROR,    .err_code = 42,      .message = "Internal Server Error. %s" },
    {.key = "not-authorized",           .http_code = HTTP_UNAUTHORIZED,             .err_code = 43,      .message = "You are not authenticated or your rights are insufficient."},
    {.key = "element-not-found",        .http_code = HTTP_NOT_FOUND,                .err_code = 44,      .message = "Element '%s' not found."},
    {.key = "method-not-allowed",       .http_code = HTTP_METHOD_NOT_ALLOWED,       .err_code = 45,      .message = "Http method '%s' not allowed."},
    {.key = "request-param-required",   .http_code = HTTP_BAD_REQUEST,              .err_code = 46,      .message = "Parameter '%s' is required." },
    {.key = "request-param-bad",        .http_code = HTTP_BAD_REQUEST,              .err_code = 47,      .message = "Parameter '%s' has bad value. Received %s. Expected %s" },
    {.key = "bad-request-document",     .http_code = HTTP_BAD_REQUEST,              .err_code = 48,      .message = "Request document has invalid syntax. %s" },
    {.key = "data-conflict",            .http_code = HTTP_CONFLICT,                 .err_code = 50,      .message = "Element '%s' cannot be processed because of conflict. %s"},
    {.key = "action-forbidden",         .http_code = HTTP_FORBIDDEN,                .err_code = 51,      .message = "%s is forbidden. %s"},
    {.key = "parameter-conflict",       .http_code = HTTP_BAD_REQUEST,              .err_code = 52,      .message = "Request cannot be processed because of conflict in parameters. %s"},
    {.key = "content-too-big",          .http_code = HTTP_REQUEST_ENTITY_TOO_LARGE, .err_code = 53,      .message = "Content size is too big, maximum size is %s" },
    {.key = "not-found",                .http_code = HTTP_NOT_FOUND,                .err_code = 54,      .message = "%s does not exist." }
    } };
#undef HTTP_TEAPOT

template <size_t N>
constexpr ssize_t
_die_idx(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > N, "_die_idx asked for too big N");
    return (_errors.at(N).key == key || _errors.at(N).message == key) ? N: _die_idx<N-1>(key);
}

template <>
constexpr ssize_t
_die_idx<1>(const char* key)
{
    static_assert(std::tuple_size<_WSErrors>::value > 1 , "_die_idx asked for too big N");
    return (_errors.at(1).key == key || _errors.at(1).message == key) ? 1: 0;
}

static int
_die_asprintf(
        char **buf,
        const char* format,
        ...) __attribute__ ((format (printf, 2, 3))) __attribute__ ((__unused__));

static int
_die_asprintf(
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
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
        static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
        char *__http_die__error_message__ = NULL; \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__, "", "", "", "", "" ); \
        reply.out() << utils::json::create_error_json(__http_die__error_message__, _errors.at(__http_die__key_idx__).err_code); \
        free(__http_die__error_message__); \
        return _errors.at(__http_die__key_idx__).http_code;\
    } \
    while(0)


/**
 *  \brief http die based on _error index number
 *
 *  \param[in] idx - index to _error array
 *  \param[msg] msg - message to be printed
 *
 * idx is normalized before is used - absolute value is used for indexing, so 1
 * equals to -1.
 * If it's bigger than size of array, it's changed to 0.
 * 0 means, that there is an error hidden somewhere.
 *
 * XXX: should not we change it to http_die_be to use instance of BiosError directly?
 */
#define http_die_idx(idx, msg) \
do { \
    int64_t _idx = idx; \
    if (_idx < 0) _idx = _idx * -1; \
    if (_idx >= (int64_t)_WSErrorsCOUNT) _idx = 0; \
    if (_idx == 0) log_error("TEAPOT");\
    reply.out() << utils::json::create_error_json(msg, _errors.at(_idx).err_code);\
    return _errors.at(_idx).http_code;\
} \
while (0)

typedef struct _http_errors_t {
    uint32_t http_code;
    std::vector <std::pair <uint32_t, std::string>> errors;
} http_errors_t;

#define http_add_error(errors, key, ...) \
do { \
    static_assert (std::is_same <decltype (errors), http_errors_t&>::value, "'errors' argument in macro http_add_error must be a http_errors_t."); \
    constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
    static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
    (errors).http_code = _errors.at (__http_die__key_idx__).http_code; \
    char *__http_die__error_message__ = NULL; \
    _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__, "", "", "", "", "" ); \
    (errors).errors.push_back (std::make_pair (_errors.at (__http_die__key_idx__).err_code, __http_die__error_message__)); \
    free (__http_die__error_message__); \
} \
while (0)

#define http_die_error(errors) \
do { \
    static_assert (std::is_same <decltype (errors), http_errors_t>::value, "'errors' argument in macro http_add_error must be a http_errors_t."); \
    reply.out() << utils::json::create_error_json ((errors).errors); \
    return (errors).http_code; \
} \
while (0)

/**
 * \brief general bios error
 *
 * \param[in] idx - index to _errors array
 * \param[in] message - error message to be printed
 *
 * This exception is not supposed to be created manually as it's easy to
 * misstype it. The bios_throw macro below is supposed to be used.
 *
 * Only one good case is to throw an error from low level function, euther via
 * return + message string or via db_reply_t or reply_t types.
 */
struct BiosError : std::invalid_argument {
    explicit BiosError (
            size_t idx,
            const std::string& message):
        std::invalid_argument(message),
        idx(idx)
    { }

    size_t idx;
};

/*
 * \brief get the index to error message and formatted error message
 *
 * \param[out] idx - variable name to store index
 * \param[out] str - variable name to store formatted error message
 * \param[in]  key - the .key or .message from static list of errors
 * \param[in]  ... - format arguments for .message template
 *
 * It is supposed for DB functions to return errors easily expressed in REST API
 *
 * // new low level DB API:
 * int db_foo_bar(conn, id, std::string &err) {
 *   ...
 *   if (!failed)
 *     return 0;
 *   ...
 *   int idx;
 *   bios_error_idx(idx, err, "internal-error");
 *   return -idx;
 * }
 *
 * // old low level DB API:
 * db_reply_t db_ham_spam(conn, id) {
 *   ...
 *   if (!failed)
 *     return ret;
 *   ...
 *   bios_error_idx(ret.rowid, ret.msg, "internal-error");
 *   return ret;
 * }
 *
 * // in REST API
 * int r = db_foo_bar(...);
 * if (r == 0)
 *   return HTTP_OK;
 *
 * http_die_idx(idx, message);
 *
 */

#define bios_error_idx(idx, str, key, ...) \
do { \
    static_assert (std::is_same <decltype (str), std::string>::value || std::is_same <decltype (str), std::string&>::value, "'str' argument in macro bios_error_idx must be a std::string."); \
    constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
    static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
    char *__http_die__error_message__ = NULL; \
    _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__, "", "", "", "", "" ); \
    str = __http_die__error_message__; \
    idx = __http_die__key_idx__; \
    free (__http_die__error_message__); \
} \
while (0)

/**
 * \brief throw specified bios error
 *
 * \param[in] key - the .key or .message from static list of errors
 * \param[in] ... - format arguments for .message template
 *
 * Similar to http_die, just throws BiosError, where it can be easily 'consumed'
 * by http_die_idx macro.
 *
 */

#define bios_throw(key, ...) \
    do { \
        constexpr size_t __http_die__key_idx__ = _die_idx<_WSErrorsCOUNT-1>((const char*)key); \
        static_assert(__http_die__key_idx__ != 0, "Can't find '" key "' in list of error messages. Either add new one either fix the typo in key"); \
        char *__http_die__error_message__ = NULL; \
        _die_asprintf(&__http_die__error_message__, _errors.at(__http_die__key_idx__).message, ##__VA_ARGS__, "", "", "", "", "" ); \
        std::string str{__http_die__error_message__}; \
        free(__http_die__error_message__); \
        log_warning("throw BiosError{%zu, \"%s\"}", __http_die__key_idx__, str.c_str());\
        throw BiosError{__http_die__key_idx__, str}; \
    } while (0);


// General template for whether type T (a standard container) is iterable
// We deliberatelly don't want to solve this for general case (we don't need it)
// If we ever need a general is_container - http://stackoverflow.com/a/9407521
template <typename T>
struct is_iterable {
      static const bool value = false;
};

// Partial specialization for std::vector
template <typename T,typename Alloc>
struct is_iterable<std::vector <T,Alloc> > {
      static const bool value = true;
};

// Partial specialization for std::list
template <typename T,typename Alloc>
struct is_iterable<std::list <T,Alloc> > {
      static const bool value = true;
};


namespace utils {

/*!
 \brief Convert string to element identifier

 \throws std::out_of_range      When number represented by 'string' is out of <1, UINT_MAX> range
 \throws std::invalid_argument  When 'string' does not represent a number 
 \return element identifier or throws
*/
uint32_t string_to_element_id (const std::string& string);

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

std::string jsonify (double t);

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
        , typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
std::string jsonify (const T& t) {
    try {
        return std::string ("\"").append (escape (t)).append ("\"");
    } catch (...) {
        return "";
    }
}

template <typename T
        , typename std::enable_if<is_iterable<T>::value>::type* = nullptr>
std::string jsonify (const T& t) {
    try {
        std::string result = "[ ";
        bool first = true;
        for (const auto& item : t) {
            if (first) {
                result += jsonify (item);
                first = false;
            }
            else {
                result += ", " + jsonify (item);
            }
        }
        result += " ]";
        return result;
    } catch (...) {
        return "[]";
    }
}

template <typename S
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_convertible<T, std::string>::value>::type* = nullptr>
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
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::string jsonify (T key, const S& value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

template <typename T
        , typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
std::string jsonify (T key, T value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

template <typename S
        , typename std::enable_if<std::is_convertible<S, std::string>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<is_iterable<T>::value>::type* = nullptr>
std::string jsonify (const S& key, const T& value) {
    return std::string (jsonify (key)).append (" : ").append (jsonify (value));
}

template <typename S
        , typename std::enable_if<is_iterable<S>::value>::type* = nullptr
        , typename T
        , typename std::enable_if<std::is_arithmetic<T>::value>::type* = nullptr>
std::string jsonify (T key, const S& value) {
    return std::string ("\"").append (jsonify (key)).append ("\" : ").append (jsonify (value));
}

std::string
create_error_json (const std::string& message, uint32_t code);

std::string
create_error_json (std::vector <std::pair<uint32_t, std::string>> messages);

} // namespace utils::json
} // namespace utils

#endif // SRC_SHARED_WEB_UTILS_H_