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
 * \file outage.cc
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \author Jim Klimov <JimKlimov@Eaton.com>
 * \brief Not yet documented file
 */
#include "calculation.h"
#include "preproc.h"

int main (UNUSED_PARAM int argc, UNUSED_PARAM char *argv [])
{
    persist::compute_new_dc_outage();
}
