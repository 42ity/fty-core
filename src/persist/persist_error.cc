/*
Copyright (C) 2014 Eaton
 
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

/*! \file persist_error.cc
 *  \brief Error handling class
    \author Alena Chernikava <alenachernikava@eaton.com>
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
    : BiosError("An element is not a device")
    { 
    }
}
