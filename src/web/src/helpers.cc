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

#include <cassert>
#include <cxxtools/regex.h>
#include <unistd.h> // make "readlink" available on ARM
#include "utils_web.h"
#include "helpers.h"
#include "str_defs.h" // EV_LICENSE_DIR, EV_DATA_DIR

#include "log.h"

const char* UserInfo::toString() {
    switch (_profile) {
        case  BiosProfile::Dashboard: return "Dashboard";
        case  BiosProfile::Admin: return "Administrator";
        case  BiosProfile::Anonymous: return "Anonymous";
    }
    /* Currently one use-case is to return a string in profiles.
       This routine could reasonably return NULL as an error here,
       but it should not reach this point anyway (return is here
       mostly for syntax purposes and to quiesce compiler warnings.
     */
    return "N/A";
}


bool
check_element_identifier (const char *param_name, const std::string& param_value, uint32_t& element_id, http_errors_t& errors) {
    assert (param_name);
    if (param_value.empty ()) {
        http_add_error ("", errors,"request-param-required", param_name);
        return false;
    }

    uint32_t eid = 0;
    try {
        eid = utils::string_to_element_id (param_value);
    }
    catch (const std::invalid_argument& e) {
        http_add_error ("", errors, "request-param-bad", param_name,
            std::string ("value '").append (param_value).append ("'").append (" is not an element identifier").c_str (),
            std::string ("an unsigned integer in range 1 to ").append (std::to_string (UINT_MAX)).append (".").c_str ());
        return false;
    }
    catch (const std::out_of_range& e) {
        http_add_error ("", errors, "request-param-bad", param_name,
            std::string ("value '").append (param_value).append ("'").append (" is out of range").c_str (),
            std::string ("value in range 1 to ").append (std::to_string (UINT_MAX)).append (".").c_str ());
        return false;
    }
    catch (const std::exception& e) {
        log_error ("std::exception caught: %s", e.what ());
        http_add_error ("", errors, "internal-error");
        return false;
    }
    element_id = eid;
    return true;
}

typedef int (t_check_func)(int letter);

bool
check_func_text (const char *param_name, const std::string& param_value, http_errors_t& errors,  size_t minlen, size_t maxlen, t_check_func func) {
    if (param_value.size () < minlen) {
        http_add_error ("", errors, "request-param-bad", param_name,
                        std::string ("value '").append (param_value).append ("'").append (" is too short").c_str (),
                        std::string ("string from ").append (std::to_string (minlen)).append (" to ").append (std::to_string(maxlen)).append (" characters.").c_str ());
        return false;
    }
    if (param_value.size () > maxlen) {
        http_add_error ("", errors, "request-param-bad", param_name,
                        std::string ("value '").append (param_value).append ("'").append (" is too long").c_str (),
                        std::string ("string from ").append (std::to_string (minlen)).append (" to ").append (std::to_string(maxlen)).append (" characters.").c_str ());
        return false;
    }
    for (const auto letter : param_value) {
        if (!func (letter)) {
        http_add_error ("", errors, "request-param-bad", param_name,
                        std::string ("value '").append (param_value).append ("'").append (" contains invalid characters").c_str (),
                        "valid string");
        return false;

        }

    }
    return true;
}

bool
check_regex_text (const char *param_name, const std::string& param_value, const std::string& regex, http_errors_t& errors)
{
    cxxtools::Regex R (regex, REG_EXTENDED | REG_ICASE);
    if (! R.match (param_value)) {
        http_add_error ("", errors, "request-param-bad", param_name,
                        std::string ("value '").append (param_value).append ("'").append (" is not valid").c_str (),
                        std::string ("string matching ").append (regex).append (" regular expression").c_str ());
        return false;
    }
    return true;
}

// 1    contains chars from 'exclude'
// 0    does not
// -1   error (not a utf8 string etc...)
int
utf8_contains_chars (const std::string& input, const std::vector <char>& exclude)
{
    // for now works with (excludes) 1 byte (ascii) chars only (_@% etc...)
    // easily extendable to exclude >1 byte characters (unicode chars) if needed

    unsigned int pos = 0;

    while (pos < input.length ()) {        
        const char c = input [pos];
        if ((c & 0x80 ) == 0) {     // lead bit is zero, must be a single ascii
            for (const auto& item : exclude)
                if (c == item)
                    return 1;
            pos = pos + 1;
        }
        else
        if ((c & 0xE0 ) == 0xC0 ) {  // 110x xxxx (2 octets)
            pos = pos + 2;
        }
        else
        if ((c & 0xF0 ) == 0xE0 ) { // 1110 xxxx (3 octets)
            pos = pos + 3;
        }
        else
        if ((c & 0xF8 ) == 0xF0 ) { // 1111 0xxx (4 octets)
            pos = pos + 4;
        }
        else {
            log_error ("Unrecognized utf8 lead byte '%x' in string '%s'", c, input.c_str ());
            return -1;
        }
    }

    return 0;
}

static const std::vector <char> single_byte_excludes = {
    '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x07', '\x08', '\x09',
    '\x0a', '\x0b', '\x0c', '\x0d', '\x0e', '\x0e', '\x0f',
    '\x22', // "
    '\x24', // $
    '\x25', // %
    '\x27', // '
    '\x28', // (
    '\x29', // )
    '\x2a', // *
    '\x2f', // SOLIDUS
    '\x40', // @
    '\x5b', // [
    '\x5c', // REVERSE SOLIDUS
    '\x5d', // ]
    '\x5e', // ^
    '\x5f', // _
    '\x60', // `
    '\x7b', // {
    '\x7c', // |
    '\x7d', // }
    '\x7e', // ~
};

bool check_asset_name (const std::string& param_name, const std::string& name, http_errors_t &errors) {
    if (utf8_contains_chars (name, single_byte_excludes) == 1) {
        http_add_error ("", errors, "request-param-bad", param_name.c_str (), name.c_str (), "valid asset name (characters not allowed: \"$%'()*/@[\\]^_`{|}~)");
        return false;
    }
    return true;
}

static bool
s_isDELETE (const tnt::HttpRequest &request)
{
    return request.getMethod () == "DELETE";
}

static bool
s_isPUT (const tnt::HttpRequest &request)
{
    return request.getMethod () == "PUT";
}

static bool
s_in (const std::string &haystack, char needle)
{
    return haystack.find (needle) != std::string::npos;
}

void
check_user_permissions (
        const UserInfo &user,
        const tnt::HttpRequest &request,
        const std::map <BiosProfile, std::string> &permissions,
        const std::string debug,
        http_errors_t &errors
        )
{
    http_errors_t error;

    if (permissions.count (user.profile ()) != 1) {
        // actually it is not an error :)
        log_info ("Permission not defined for given profile");
        http_add_error (debug, errors, "not-authorized");
        return;
    }

    const std::string perm = permissions.at (user.profile ());

    if (  (request.isMethodGET  () && s_in (perm, 'R'))
        ||(request.isMethodPOST () && (s_in (perm, 'C') || s_in (perm, 'E')))
        ||(s_isPUT (request)       && s_in (perm, 'U'))
        ||(s_isDELETE (request)    && s_in (perm, 'D'))
       )
    {
        errors.http_code = HTTP_OK;
        return;
    }

    http_add_error (debug, errors, "not-authorized");
    return;
}

char*
get_current_license_file (void)
{
    char *current_license = NULL;
    char *env = getenv (EV_LICENSE_DIR);

    int rv = asprintf (&current_license, "%s/current", env ? env : "/usr/share/bios/license");
    if ( rv == -1 ) {
        return NULL;
    }
    return current_license;
}

char*
get_accepted_license_file (void)
{
    char *accepted_license = NULL;
    char *env = getenv (EV_DATA_DIR);

    if (asprintf (&accepted_license, "%s/license", env ? env : "/var/lib/bios" ) == -1) {
        return NULL;
    }
    return accepted_license;
}
char*
get_current_license_version (const char* license_file)
{
    // ASSUMPTION: the symlink to the text of the licence is: /XXX
    // $ ls -l /XXX
    // lrwxrwxrwx. 1 achernikava achernikava 3 Sep 25  2015 /XXX -> 1.0
    //
    // FYI:
    // readlink() places the contents of the symbolic link pathname in the
    // buffer buf, which has size bufsiz.  readlink() does not append a null
    // byte to buf.  It will truncate the contents (to a length of bufsiz
    // characters), in case the buffer is too small to hold all of the
    // contents.
    //
    // ssize_t readlink(const char *pathname, char *buf, size_t bufsiz);

    char *buff = (char *) malloc (sizeof(char)*513);
    memset(buff, 0, sizeof(char)*513);
    int rv = readlink (license_file, buff, sizeof(char)*512);
    //
    // On success, these calls return the number of bytes placed in buf.
    // On error, -1 is returned and errno is set to indicate the error.
    //
    if ( rv == -1 ) {
        log_error ("Cannot read symlink for license");
        free (buff);
        return NULL;
    }
    buff[rv] = '\0';
    return buff;
}
// drop the last / in a developer friendly way
// this is intended to fix issue we've on rhel
// "version" : "/usr/share/bios/license/1.0"
// if there's no / in inp, then it's noop
// if so, then it returns character AFTER last /
const char*
basename2 (const char *inp)
{
    const char *sep = strrchr (inp, '/');
    if (!sep)
        return inp;
    return sep + 1;
}
