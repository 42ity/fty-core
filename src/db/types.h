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

/*! \file   types.h
    \brief  Header file for types used in db api visible to the other code components/modules.
    \author Karol Hrdina <KarolHrdina@eaton.com>
*/

#ifndef SRC_DB_TYPES_H__
#define SRC_DB_TYPES_H__

struct reply_t {
    int rv; // return value
    uint64_t row_id; // row id 
    uint64_t affected_rows; // number of rows affected
};

#endif // SRC_DB_TYPES_H__

