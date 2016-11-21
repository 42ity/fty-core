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
 * \file   helpers.h
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \brief  REST helpers
 */
#ifndef SRC_WEB_INCLUDE_HELPERS_H_
#define SRC_WEB_INCLUDE_HELPERS_H_

#include <string>
#include "utils_web.h"
#include <tnt/httprequest.h>

/*!
 \brief BiosProfile enum - defines levels of permissions

    XXX: note that casting to int does not make a sense here as "higher" profile is
    supposed to be a superset of a "lower" one. This is for compatibility reason
    and the best is to assign meaningless constants here.

*/
enum struct BiosProfile {
    Anonymous = -1,             // Not authorized users
    Dashboard = 0,              // Dashboard profile
    Admin = 3                   // Admin profile
};

/*!
 \brief UserInfo class

 stores various information about User

*/
class UserInfo {

    public:
        /*!\brief default constructor for UserInfo
        */
        UserInfo ():
            _profile {BiosProfile::Anonymous},
            _uid {-1},
            _gid {-1}           
        {};

        BiosProfile profile () const {return _profile;}
        void profile (BiosProfile profile) {_profile = profile;}

        long int uid () const {return _uid;}
        void uid (long int uid) {_uid = uid;}

        long int gid () const {return _gid;}
        void gid (long int gid) {_gid = gid;}
        
        std::string login () const {return _login;}
        void login (const std::string& login) {_login = login;}

        const char* toString(); 
        
    protected:
        BiosProfile _profile;
        long int _uid;
        long int _gid;
        std::string _login;
};

// 1    contains chars from 'exclude'
// 0    does not
// -1   error (not a utf8 string etc...)
int
utf8_contains_chars (const std::string& input, const std::vector <char>& exclude);

/*!
 \brief Perform error checking and extraction of element identifier from std::string

 \param[in]     param_name      name of the parameter from rest api call
 \param[in]     param_value     value of the parameter 
 \param[out]    element_id      extracted element identifier
 \param[out]    errors          errors structure for storing conversion errors
 \return
    true on success, element_id is assigned the converted element identifier
    false on failure, errors are updated (exactly one item is added to structure)
*/
bool 
check_element_identifier (const char *param_name, const std::string& param_value, uint32_t& element_id, http_errors_t& errors);

/*!
  \brief macro for typical usage of check_element_identifier. Webserver dies with bad-param if
         the check fails
  \param[in]     name            name of the parameter from rest api call
  \param[in]     fromuser        variable containing string comming from user/network
  \param[out]    checked         variable fo be assigned with checked content
*/
#define check_element_identifier_or_die(name, fromuser, checked) \
{  \
    http_errors_t errors; \
    if (! check_element_identifier (name, fromuser, checked, errors)) { \
        http_die_error (errors); \
    } \
}

/*!
  \brief Check whether string matches regexp (case insensitive, extended regexp).
*/
bool
check_regex_text (const char *param_name, const std::string& param_value, const std::string& regex, http_errors_t& errors);

/*!
  \brief macro for typical usage of check_regex_text. Webserver dies with bad-param if
         the check fails
  \param[in]     name            name of the parameter from rest api call
  \param[in]     fromuser        variable containing string comming from user/network
  \param[out]    checked         variable fo be assigned with checked content
  \param[in]     regex           regex (extended|icase) for variable checking
*/
#define check_regex_text_or_die(name, fromuser, checked, regexp) \
{  \
    http_errors_t errors; \
    if (check_regex_text (name, fromuser, regexp, errors)) { \
        checked = fromuser; \
    } else { \
        http_die_error (errors); \
    } \
}

#define _ALERT_RULE_NAME_RE_STR "^[-_.a-z0-9@]{1,255}$"
/*!
  \brief macro to check the alert name
  \param[in]     name            name of the parameter from rest api call
  \param[in]     fromuser        variable containing string comming from user/network
  \param[out]    checked         variable fo be assigned with checked content
*/

#define check_alert_rule_name_or_die(name, fromuser, checked) \
{  \
    http_errors_t errors; \
    if (check_regex_text (name, fromuser, _ALERT_RULE_NAME_RE_STR, errors)) { \
        checked = fromuser; \
    } else { \
        http_die_error (errors); \
    } \
}

// checks '<rule_name>@<asset_name>' format 
bool check_alert_rule_name (const std::string& param_name, const std::string& rule_name, http_errors_t& errors);

// checks just rule name  
bool check_alert_just_rule_part (const std::string& param_name, const std::string& rule_name, http_errors_t& errors);

/*!
 \brief Check valid asset name
 \param[in]     param_name      name of the parameter from rest api call
 \param[in]     name            given name of asset
 \param[out]    errors          errors structure for storing conversion errors
 \return
    true on success
    false on failure, errors are updated (exactly one item is added to structure)

*/
bool check_asset_name (const std::string& param_name, const std::string& name, http_errors_t &errors);

/*!
 * \brief Check user permissions
 *
 * \param [in]  user            request variable user
 * \param [in]  request         http request
 * \param [in]  permissions     maps permissions with profile
 * \param [in]  debug           user provided "debug" object for JSON
 *
 * Permissions is map of BiosProfile and string encoding available permissions,
 * where
 *  "C" means Create / POST method
 *  "R" means Read / GET method
 *  "U" means Update / PUT method
 *  "D" means Delete / DELETE method
 *  "E" means Execute / POST method
 *
 *  other strings are silently ignored
 *
 * Example:
 *
 * check_user_permissions (user, request, {
 *      {BiosProfile::Anonymous, "R"},
 *      {BiosProfile::Dashboard, "CR"},
 *      {BiosProfile::Admin, "CRUDE"}
 * });
 *
 */
void check_user_permissions (
        const UserInfo &user,
        const tnt::HttpRequest &request,
        const std::map <BiosProfile, std::string> &permissions,
        const std::string debug,
        http_errors_t &errors
        );

#define CHECK_USER_PERMISSIONS_OR_DIE(p) \
    do { \
        http_errors_t errors; \
        std::string __http_die__debug__ {""}; \
        if (::getenv ("BIOS_LOG_LEVEL") && !strcmp (::getenv ("BIOS_LOG_LEVEL"), "LOG_DEBUG")) { \
            __http_die__debug__ = {__FILE__}; \
            __http_die__debug__ += ": " + std::to_string (__LINE__); \
        } \
        check_user_permissions (user, request, p, __http_die__debug__, errors);\
        if (errors.http_code != HTTP_OK) \
            http_die_error (errors);\
    } while (0)

#endif // SRC_WEB_INCLUDE_HELPERS_H_

// Helper function to work with license
char* get_current_license_file (void);
char* get_accepted_license_file (void);
char* get_current_license_version (const char* license_file);
const char* basename2 (const char *inp);
