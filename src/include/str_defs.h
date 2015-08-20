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
 * \file str_defs.h
 * \author Karol Hrdina
 * \brief Not yet documented file
 */
#ifndef SRC_INCLUDE_STR_DEFS_H__
#define SRC_INCLUDE_STR_DEFS_H__

// bios malamute endpoint
extern const char* MLM_ENDPOINT;

// names of malamute clients (== bios agents)
extern const char* BIOS_AGENT_NAME_COMPUTATION;
extern const char* BIOS_AGENT_PREFIX_REST; // each client created inside tntnet is suffixed by it's pid (getpid ()).
extern const char* BIOS_AGENT_NAME_DB_MEASUREMENT;

#define AVG_STEPS_SIZE 5
extern const char* AVG_STEPS[AVG_STEPS_SIZE];
#define AVG_TYPES_SIZE 3
extern const char* AVG_TYPES[AVG_TYPES_SIZE];

extern const char* DATETIME_FORMAT;
#define DATETIME_FORMAT_LENGTH 15

extern const char* STRFTIME_DATETIME_FORMAT;

// protocol related
extern const char* BIOS_WEB_AVERAGE_REPLY_JSON_TMPL;
extern const char* BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL;

// evironment variables
extern const char* EV_BIOS_LOG_LEVEL;

#endif // SRC_INCLUDE_STR_DEFS_H__

