#!/bin/bash

#
# Copyright (C) 2015 - 2020 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


#! \file run-test.sh
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author Alena Chernikava <AlenaChernikava@Eaton.com>
#  \brief Not yet documented file

NEWARGS="${@:1:$#-1}"
NEWPROGRAM="${@:$#:$#}"
declare -r HARD_ERROR=99
if [[ -z "${NEWPROGRAM}" ]]; then
    echo "FATAL: name of test is missing, exiting!"
    exit ${HARD_ERROR}
fi

mkdir -p tests/junit/

exec "${NEWPROGRAM}" "${NEWARGS}" -r junit -o "tests/junit/${1}.xml"
