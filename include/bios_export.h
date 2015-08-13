/*
Copyright (C) 2014 - 2015 Eaton

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

/*! \file   bios_export.h
    \brief  Bios public application interface export macro
    \author Karol Hrdina <KarolHrdina@eaton.com>
    \author Alena Chernikava <alenachernikava@eaton.com>
    \author Tomas Halman <TomasHalman@eaton.com>
*/

#ifndef INCLUDE_BIOS_EXPORT_H__
#define INCLUDE_BIOS_EXPORT_H__

// These macros allow to mark certain functions or variables in the headers
// for $BIOS Project public API as "hidden" or "default". See details at:
// http://gcc.gnu.org/onlinedocs/gcc-4.9.2/gcc/Function-Attributes.html#index-g_t_0040code_007bvisibility_007d-attribute-3030
// You don't need to (or rather should not) duplicate these in the .c files.

// Note that these macros are only assigned some values during a build of
// libbiosapi.{so,a} and are empty otherwise.

// For certain items, contradict our default of GCC "-fvisibility=hidden"
#ifndef BIOS_EXPORT
# if BUILDING_LIBBIOSAPI && HAVE_VISIBILITY
#  define BIOS_EXPORT __attribute__((__visibility__("default")))
# else
#  define BIOS_EXPORT
# endif
#endif

// For certain items, enforce "-fvisibility=hidden"
#ifndef BIOS_HIDDEN
# if BUILDING_LIBBIOSAPI && HAVE_VISIBILITY
#  define BIOS_HIDDEN __attribute__((__visibility__("hidden")))
# else
#  define BIOS_HIDDEN
# endif
#endif

#endif // INCLUDE_BIOS_EXPORT_H__

