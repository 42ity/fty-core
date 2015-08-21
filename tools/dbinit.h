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
 * \file dbinit.h
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Karol Hrdina <KarolHrdina@Eaton.com>
 * \brief Not yet documented file
 */
#ifndef DBINIT_H_
#define DBINIT_H_

#include "../src/persist/databaseobject.h"
#include "dbpath.h"

static const std::string osnew       = persist::objectStatetoString(persist::ObjectState::OS_NEW);
static const std::string osdeleted   = persist::objectStatetoString(persist::ObjectState::OS_DELETED);
static const std::string osupdated   = persist::objectStatetoString(persist::ObjectState::OS_UPDATED);
static const std::string osselected  = persist::objectStatetoString(persist::ObjectState::OS_SELECTED);
static const std::string osinserted  = persist::objectStatetoString(persist::ObjectState::OS_INSERTED);


#endif //DBINIT_H_
