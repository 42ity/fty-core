#!/bin/sh

#
#   Copyright (c) 2020 Eaton
#
#   This file is part of the Eaton 42ity project.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    999-bios-pre-eula-target.everytime.sh
#  \brief   Make sure this deployment does run all configured constituents of bios-pre-eula.target
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#
# Note: as services get delivered by ipc-meta-setup snippets,
# there arises a need to make sure they to run when expected.
#

/bin/systemctl start --no-block bios-pre-eula.target || true
