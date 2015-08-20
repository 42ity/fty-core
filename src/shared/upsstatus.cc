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
 * \file upsstatus.cc
 * \author Tomas Halman
 * \author Karol Hrdina
 * \brief Not yet documented file
 */
#include "upsstatus.h"
#include <string.h>
#include <iostream>
#include "utils.h"
#include "cleanup.h"

namespace shared {

/* following definition is taken as it is from network ups tool project (dummy-ups.h)*/

/**
 * Status lookup table
 */
status_lkp_t status_info[] = {
    { "CAL", STATUS_CAL },
    { "TRIM", STATUS_TRIM },
    { "BOOST", STATUS_BOOST },
    { "OL", STATUS_OL },
    { "OB", STATUS_OB },
    { "OVER", STATUS_OVER },
    { "LB", STATUS_LB },
    { "RB", STATUS_RB },
    { "BYPASS", STATUS_BYPASS },
    { "OFF", STATUS_OFF },
    { "CHRG", STATUS_CHRG },
    { "DISCHRG", STATUS_DISCHRG },
    { "HB", STATUS_HB },
    { "FSD", STATUS_FSD },    
    { "NULL", 0 },
};
/* previous definition is taken as it is from network ups tool project (dummy-ups.h)*/


uint16_t upsstatus_single_status_to_int(char *status) {
    if( ! status ) return 0;
    int i = 0;
    while(true) {
        if( status_info[i].status_value == 0 ) {
            // end of array, not found
            return 0;
        }
        if( strncasecmp(status_info[i].status_str, status, strlen(status_info[i].status_str) ) == 0 ) {
            return status_info[i].status_value;
        }
        i++;
    }
}

uint16_t upsstatus_to_int(const char *status) {
    uint16_t result = 0;
    _scoped_char *buff = strdup(status);
    char *b = buff;
    char *e;

    if(!buff) {
        return 0;
    }
    while(b) {
        e = strchr(b,' ');
        if(e) {
            *e = 0;
            e++;
        }
        result |= upsstatus_single_status_to_int(b);
        b = e;
    }
    FREE0 (buff)
    return result;
}

uint16_t upsstatus_to_int(const std::string &status) {
    return upsstatus_to_int(status.c_str());
}

std::string upsstatus_to_string(uint16_t status) {
    std::string result = "";
    int bit = 1;
    for( unsigned int i = 0 ; i < sizeof(status_info)/sizeof(status_lkp_t) - 1 ; ++i ) {
        if( status & bit ) {
            if( result.length() ) {
                result += " ";
            }
            result += status_info[i].status_str;
        }
        bit <<= 1;
    }
    return result;
}

std::string upsstatus_to_string(std::string status) {
    return upsstatus_to_string( atoi( status.c_str() ) );
}


} // namespace shared
