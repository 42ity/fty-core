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

/*! \file   assetu.cc
    \brief  Basic update-functions for assets
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#include <tntdb/connect.h>
#include "dbtypes.h"

#ifndef  SCR_DB_ASSETS_ASSETU
#define  SCR_DB_ASSETS_ASSETU

namespace persist{

//+
// name would not be updated, but to provide a functionality for future
// parameter was added
int
    update_asset_element
        (tntdb::Connection &conn,
         a_elmnt_id_t     element_id,
         const char      *element_name,
         a_elmnt_id_t     parent_id,
         const char      *status,
         a_elmnt_pr_t     priority,
         a_elmnt_bc_t     bc,
         const char      *asset_tag,
         int32_t         &affected_rows);


} //name space end

#endif  // SCR_DB_ASSETS_ASSETU
