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
 * \file sample_agent.h
 * \author Michal Hrusecky
 * \author Alena Chernikava
 * \brief Not yet documented file
 */
#ifndef SRC_SHARED_SAMPLE_AGENT_H
#define SRC_SHARED_SAMPLE_AGENT_H

#include <sys/types.h>

#include "bios_agent.h"
#include "agents.h"

struct sample_agent {
    const char* agent_name;   //!< Name of the measuring agent
    int (*init)();            //!< Constructor 
    int (*close)();           //!< Destructor
    char **variants;          //!< Various sources to iterate over
    const char* measurement;  /*!< Printf formated string for what are we
                                   measuring, %s will be filled with source */
    const char* at;           /*!< Printf formated string for where are we
                                   measuring, %s will be filled with hostname */
    int32_t diff;             /*!< Minimum difference required for publishing */
    ymsg_t* (*get_measurement)(char* what); //!< Measuring itself
};

// extern sample_agent agent;
#endif // SRC_SHARED_SAMPLE_AGENT_H
