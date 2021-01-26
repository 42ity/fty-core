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
#! \file   license-agreement.sh
#  \brief  agree with EULA for a user
#  \note  restapi-request.sh dep.

set +x

abort()
{
  stty echo
  echo "Canceled"
  exit 1
}

trap abort SIGINT

HOST="localhost"
USER="admin"

while [ $# -gt 0 ] ; do
    case "$1" in
        --help)
            echo "Usage: $(basename $0) [options...]"
            echo "   --host|-h <hostname> (default: localhost)"
            echo "   --user|-u <username> (default: admin)"
            echo "   --help"
            exit 1
            ;;
        --user|-u)
            USER="$2"
            shift
            ;;
        --host|-h)
            HOST="$2"
            shift
            ;;
        *)  echo "Unrecognized params follow: $*" >&2
            exit 1
            ;;
    esac
    shift
done

echo "Confirm that you agree with the EULA by entering password for user '$USER' (<CTRL+C> to cancel):"
read -s PASSWORD

./restapi-request.sh -u "$USER" -p "$PASSWORD" --host "$HOST" --use-https -m accept_license

exit 0
