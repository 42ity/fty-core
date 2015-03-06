/*
Copyright (C) 2014 - 2015 Eaton

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

/*!
 \file   bios_export.h
 \brief  Bios public application interface export macro
*/

#ifndef INCLUDE_BIOS_EXPORT_H__
#define INCLUDE_BIOS_EXPORT_H__

// For certain items, contradict the default of GCC "-fvisibility=hidden"
#ifndef BIOS_EXPORT
# if BUILDING_LIBBIOSAPI && HAVE_VISIBILITY
#  define BIOS_EXPORT __attribute__((__visibility__("default")))
# else
#  define BIOS_EXPORT
# endif
#endif

#endif // INCLUDE_BIOS_EXPORT_H__

