#!/bin/bash

#
# Copyright (C) 2014 - 2020 Eaton
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


#! \file   generate_env_user4sql.sh
#  \author Xavier Millieret <XavierMillieret@Eaton.com>
#  \brief  Set env varaibles for sql script.

set -e

if [ $# -eq 2 ]
then
    while [ "$#" -gt 0 ]; do
        case "$1" in
            -O) OUTFILE="$2";  shift;;
            *)  echo "Unknown argument: $1" >&2 ; exit 1 ;;
        esac
        shift
    done
else
    echo "Invalid argument please provide the output file name"
    exit -1
fi

echo "Generate environment variables for user management"

# Get user's uid
UID_ADMIN=`id -u admin`
UID_MONITOR=`id -u monitor`

cat << EOF > "$OUTFILE"
SET @ENV_ADMIN=${UID_ADMIN};
SET @ENV_MONITOR=${UID_MONITOR};
EOF
exit 0
