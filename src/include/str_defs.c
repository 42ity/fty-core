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
 * \file str_defs.c
 * \author Karol Hrdina
 * \author Jim Klimov
 * \brief Not yet documented file
 */
#include "str_defs.h"

/* Implement variables declared in str_defs.h */

const char* MLM_ENDPOINT = "ipc://@/malamute";

const char *BIOS_AGENT_NAME_COMPUTATION = "agent-cm";
const char *BIOS_AGENT_PREFIX_REST = "rest.";
const char *BIOS_AGENT_NAME_DB_MEASUREMENT = "persistence.measurement";

const char *AVG_STEPS[AVG_STEPS_SIZE] = {
    "15m",
    "30m",
    "1h",
    "8h",
    "24h"
};

const char *AVG_TYPES[AVG_TYPES_SIZE] = {
    "arithmetic_mean",
    "min",
    "max"
};

const char* DATETIME_FORMAT = "%4d%2d%2d%2d%2d%2d%c";

const char* BIOS_WEB_AVERAGE_REPLY_JSON_TMPL =
    "{\n"
    "\"units\": \"##UNITS##\",\n"
    "\"source\": \"##SOURCE##\",\n"
    "\"step\": \"##STEP##\",\n"
    "\"type\": \"##TYPE##\",\n"
    "\"element_id\": ##ELEMENT_ID##,\n"
    "\"start_ts\": ##START_TS##,\n"
    "\"end_ts\": ##END_TS##,\n"
    "\"data\": [\n"
    "##DATA##\n"
    "]}";
const char* BIOS_WEB_AVERAGE_REPLY_JSON_DATA_ITEM_TMPL =
    "\t{\n"
    "\t\t\"value\": ##VALUE##,\n"
    "\t\t\"timestamp\": ##TIMESTAMP##\n"
    "\t}";

const char* STRFTIME_DATETIME_FORMAT = "%Y%m%d%H%M%SZ";    

const char* EV_BIOS_LOG_LEVEL = "BIOS_LOG_LEVEL";

