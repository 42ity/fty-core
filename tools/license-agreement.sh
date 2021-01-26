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

canceled()
{
  stty echo
  echo "Canceled"
  exit 1
}

trap canceled SIGINT

HOST="localhost"
PORT="443"
USER="admin"
NTRY=3

while [ $# -gt 0 ] ; do
    case "$1" in
        --help)
            echo "Usage: $(basename $0) [options...]"
            echo "   --host|-h <hostname> (default: 'localhost')"
            echo "   --port|-p <port> (default: '443')"
            echo "   --user|-u <username> (default: 'admin')"
            echo "   --ntry|-n <number-of-tries> (default: 3, min: 1)"
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
        --port|-p)
            PORT="$2"
            shift
            ;;
        --ntry|-n)
            NTRY=$2
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

while true; do
    RESULT=$(/usr/share/fty/scripts/restapi-request.sh -u "$USER" -p "$PASSWORD" --host "$HOST" --port-web $PORT --use-https -m accept_license 2> /dev/null)

    ## check GET /admin/license/status
    ##curl -X GET -k -H 'Authorization: Bearer  <bearer>' -i 'https://<ip>:443/api/v1/admin/license/status'
    ##{"accepted":"yes","version":"1.0","accepted_version":"1.0","accepted_at":"1611663919","accepted_by":"admin"}
    grep "accepted_at" <<< "$RESULT" && break

    ## check POST /admin/license result
    ##curl -X POST -k -H 'Authorization: Bearer <bearer>' -i 'https://<ip>:443/api/v1/admin/license'
    ##{"success":"License version 1.0 accepted."}
    grep -q "\"success\"" <<< "$RESULT" && break

    ((NTRY--))
    if [ "$NTRY" -gt 0 ]; then
        echo "Retry..."
        continue
    fi

    echo "License is not accepted"
    exit 255
done

echo "License is accepted"
exit 0
