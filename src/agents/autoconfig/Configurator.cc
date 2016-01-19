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

/*!
 \file   Configurator.cc
 \brief  Configurator implementation
 \author Tomas Halman <TomasHalman@Eaton.com>
*/

#include "log.h"
#include "preproc.h"

#include "Configurator.h"

bool Configurator::configure (const std::string& name, const AutoConfigurationInfo& info)
{
    log_error ("Configuration of device = '%s' type = '%" PRIu32 "' subtype = '%" PRIu32"' was not implemented yet.",
               name.c_str(), info.type, info.subtype );
    return true;
}

std::vector<std::string> Configurator::createRules (UNUSED_PARAM std::string const& name) {
    std::vector<std::string> result;
    return result;
}
