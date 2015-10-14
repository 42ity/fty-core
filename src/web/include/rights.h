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
 * \file rights.h
 * \author Michal Hrusecky <MichalHrusecky@Eaton.com>
 * \brief Rights management structure
 *
 * Put here list of urls and the required access level. First entry is string
 * starting with letter "R" or "W". This denotes whether we are dealing with
 * read access or write access. It is directly followed by url. It doesn't have
 * to be full url - if part that you wrote matches, rule is applied. Longer the
 * string, the more preference it has. Access level is number from -1
 * (Unauthenticated) to 3 (Admin). You specify the least privileges user needs
 * to access this call. So for example if you specify, that
 * "W/api/v1/admin/time" requires level 2, users with level 2 and above can use
 * it even if "W/api/v1/admin" requires level 3.
 *
 */

#include <map>
#include <string>


std::map<std::string, int> rights_management = {
{ "R", -1 },
{ "W", 2 },
{ "W/api/v1/admin", 3 },
{ "R/api/v1/admin/systemctl", 2 },
{ "R/api/v1/asset/export", 2 },
{ "W/api/v1/oauth2/token", -1 },
};
