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

/*! \file   persist_error.h
    \brief  Error handling class
    \author Alena Chernikava <alenachernikava@eaton.com>
*/

#ifndef SRC_PERSIST_PERSIST_ERROR_H
#define SRC_PERSIST_PERSIST_ERROR_H

#include <stdexcept>
#include <string>

namespace bios
{
    /**
     * \brief A base class for errors.
     */
    class BiosError : public std::runtime_error
    {
        public:
        
        // Constructor
        explicit BiosError(const std::string& msg);
    };

    /*
     * \brief Exception thrown when specified element is not a device.
     */
    class ElementIsNotDevice : public BiosError
    {
        public:
        // Constructor
        ElementIsNotDevice();
    };
    
    /*
     * \brief Exception thrown when for specified device 
     *  monitor counterpart was not found.
     */
    class MonitorCounterpartNotFound : public BiosError
    {
        public:
        // Constructor
        MonitorCounterpartNotFound();
    };

    /*
     * \brief Exception thrown when speified element was not
     * found in database.
     */
    class NotFound : public BiosError
    {
        public:
        // Constructor
        NotFound();
    };

    /*
     * \brief Exception thrown when some unexcpected error in database
     * occured.
     */
    class InternalDBError : public BiosError
    {
        public:
        // Constructor
        InternalDBError(const std::string& msg);
    };
    
    /*
     * \brief Exception thrown when bad input data found
     * in database.
     */
    class BadInput : public BiosError
    {
        public:
        // Constructor
        BadInput(const std::string& msg);
    };

}
#endif // SRC_PERSIST_PERSIST_ERROR_H
