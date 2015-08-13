/*
Copyright (C) 2014 Eaton
 
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

#include "persist_error.h"

namespace bios
{
    BiosError::BiosError(const std::string& msg)
    : std::runtime_error(msg)
    {
    }
    
    NotFound::NotFound()
    : BiosError("Specified element was not found")
    { 
    }
    
    InternalDBError::InternalDBError(const std::string& msg)
    : BiosError(msg)
    { 
    }

    ElementIsNotDevice::ElementIsNotDevice()
    : BiosError("Specified element is not a device")
    { 
    }

    MonitorCounterpartNotFound::MonitorCounterpartNotFound()
    :BiosError("For specified device monitor counterpart was not found")
    {
    }
    
    BadInput::BadInput(const std::string& msg)
    : BiosError(msg)
    { 
    }

}
