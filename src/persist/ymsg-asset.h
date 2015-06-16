/*
Copyright (C) 2014-2015 Eaton
 
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

/*! \file ymsg-asset.h
    \author Tomas Halman
*/

#ifndef SRC_PERSIST_YMSG_ASSET_H__
#define SRC_PERSIST_YMSG_ASSET_H__

#include "ymsg.h"

namespace persist {

//! Processes alert message and creates an answer
void process_alert(ymsg_t* out, char** out_subj,
                   ymsg_t* in, const char* in_subj);

void process_get_asset(ymsg_t* out, char** out_subj,
                       ymsg_t* in, const char* in_subj);

void
    process_get_asset_extra
        (ymsg_t* out, char** out_subj,
         ymsg_t* in,  const char* in_subj);

}

#endif // SRC_PERSIST_YMSG_ASSET_H__

