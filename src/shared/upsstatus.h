#ifndef SRC_SHARED_UPSSTATUS_H_
#define SRC_SHARED_UPSSTATUS_H_
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

/*! \file upsstatus.h
    \brief  functions to work with ups status string representation
            as it is used in networkupstools
    \author Tomas Halman <tomashalman@eaton.com>
*/

#include<string>
#include<cstdint>

namespace shared {

/* following definition is taken as it is from network ups tool project (dummy-ups.h)*/

typedef struct {
    const char  *status_str;    /* ups.status string */
    int          status_value;  /* ups.status flag bit */
} status_lkp_t;

#define STATUS_CAL             (1 << 0)        /* calibration */
#define STATUS_TRIM            (1 << 1)        /* SmartTrim */
#define STATUS_BOOST           (1 << 2)        /* SmartBoost */
#define STATUS_OL              (1 << 3)        /* on line */
#define STATUS_OB              (1 << 4)        /* on battery */
#define STATUS_OVER            (1 << 5)        /* overload */
#define STATUS_LB              (1 << 6)        /* low battery */
#define STATUS_RB              (1 << 7)        /* replace battery */
#define STATUS_BYPASS          (1 << 8)        /* on bypass */
#define STATUS_OFF             (1 << 9)        /* ups is off */
#define STATUS_CHRG            (1 << 10)       /* charging */
#define STATUS_DISCHRG         (1 << 11)       /* discharging */

/* previous definition is taken as it is from network ups tool project (dummy-ups.h)*/

/* our transformation functions */
 
/**
 * \brief converts status from char* format to bitmap representation
 * \parameter char* string with ups status (for example "OL CHRG")
 * \return uint16_t status bitmap
 */
uint16_t upsstatus_to_int(const char *status);

/**
 * \brief converts status from char* format to bitmap representation
 * \parameter std::string string with ups status (for example "OL CHRG")
 * \return uint16_t status bitmap
 */
uint16_t upsstatus_to_int(const std::string &status);

/**
 * \brief converts status from uint16_t bitmap to text representation
 * \parameter uint16_t bitmap representing UPS status (for example STATUS_CHRG|STATUS_OL)
 * \return std::string text representation (for example "OL CHRG")
 */
std::string upsstatus_to_string(uint16_t status);

} // namespace shared
#endif // SRC_SHARED_UPSSTATUS_H_
