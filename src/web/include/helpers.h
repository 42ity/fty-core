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
            _gid {-1},
            _login {"toto"}    
        {};

        BiosProfile profile () const {return _profile;}
        void profile (BiosProfile profile) {_profile = profile;}

        long int uid () const {return _uid;}
        void uid (long int uid) {_uid = uid;}

        long int gid () const {return _gid;}
        void gid (long int gid) {_gid = gid;}
        
        std::string login () const {return _login;}
        void login (std::string login) {_login = login;}
        
    protected:
        BiosProfile _profile;
        long int _uid;
        long int _gid;
        std::string _login;
};


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

#endif // SRC_WEB_INCLUDE_HELPERS_H_

