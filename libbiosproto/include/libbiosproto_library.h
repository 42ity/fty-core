/*  =========================================================================
    libbiosproto - LIBBIOSPROTO wrapper

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of CZMQ, the high-level C binding for 0MQ:       
    http://czmq.zeromq.org.                                            
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           

################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
    =========================================================================
*/

#ifndef libbiosproto_library_H_INCLUDED
#define libbiosproto_library_H_INCLUDED

//  Set up environment for the application

//  External dependencies
#include <malamute.h>

//  LIBBIOSPROTO version macros for compile-time API detection

#define LIBBIOSPROTO_VERSION_MAJOR 1
#define LIBBIOSPROTO_VERSION_MINOR 0
#define LIBBIOSPROTO_VERSION_PATCH 0

#define LIBBIOSPROTO_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define LIBBIOSPROTO_VERSION \
    LIBBIOSPROTO_MAKE_VERSION(LIBBIOSPROTO_VERSION_MAJOR, LIBBIOSPROTO_VERSION_MINOR, LIBBIOSPROTO_VERSION_PATCH)

#if defined (__WINDOWS__)
#   if defined LIBLIBBIOSPROTO_STATIC
#       define LIBBIOSPROTO_EXPORT
#   elif defined LIBLIBBIOSPROTO_EXPORTS
#       define LIBBIOSPROTO_EXPORT __declspec(dllexport)
#   else
#       define LIBBIOSPROTO_EXPORT __declspec(dllimport)
#   endif
#else
#   define LIBBIOSPROTO_EXPORT
#endif

//  Opaque class structures to allow forward references
typedef struct _bios_proto_t bios_proto_t;
#define BIOS_PROTO_T_DEFINED


//  Public API classes
#include "bios_proto.h"

#endif
/*
################################################################################
#  THIS FILE IS 100% GENERATED BY ZPROJECT; DO NOT EDIT EXCEPT EXPERIMENTALLY  #
#  Please refer to the README for information about making permanent changes.  #
################################################################################
*/
