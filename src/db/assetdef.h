/*
Copyright (C) 2015 Eaton

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*! \file   assetdef.h
    \brief  Structure to manage assets
    \author Alena Chernikava <AlenaChernikava@eaton.com>
*/

#ifndef SRC_DB_ASSETS_ASSETDEFS_H_
#define SRC_DB_ASSETS_ASSETDEFS_H_

typedef struct _asset_link
{
    a_elmnt_id_t    src;     //!< id of src element
    a_elmnt_id_t    dest;    //!< id of dest element
    a_lnk_src_out_t src_out; //!< outlet in src
    a_lnk_src_out_t dest_in; //!< inlet in dest
    a_lnk_tp_id_t   type;    //!< link type id
} link_t;

#endif // SRC_DB_ASSETS_ASSETDEFS_H_

