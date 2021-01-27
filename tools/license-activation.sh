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
#! \file   license-activation.sh
#  \brief  CLI over etn-licensing
#  \note   malamute/bmsg dep.

set +x

canceled()
{
  stty echo
  echo "Canceled"
  exit 1
}

#return a random string of length $1 (default: 8)
srand()
{
    LEN=$1
    [ -z $LEN ] && LEN=8
    [ $LEN -le 0 ] && LEN=8
    echo $(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w $LEN | head -n 1)
}

#request etn-licensing TEST_ONLINE
#return:
#   0 if online access is available,
# 255 if online access is not available
test_online()
{
    CID=$(srand)
    RESULT=$(bmsg request etn-licensing GET REQUEST $CID TEST_ONLINE)
    SUCCESS=$(grep -z "OK\s{ \"key\"" <<< "$RESULT")

    if [ -z $SUCCESS ]; then
        echo "Internet access not available"
        return 255
    fi
    echo "Internet access available"
    return 0
}

#request etn-licensing ACTIVATE_ONLINE
#return:
#   0 if activation success,
# 255 if activation failed,
#   1 if activationID argument ($1) is missing
activate_online()
{
    ACTIVATION_ID=$1
    if [ -z $ACTIVATION_ID ]; then
        echo "Activation ID is missing"
        return 1
    fi

    CID=$(srand)
    RESULT=$(bmsg request etn-licensing GET REQUEST $CID ACTIVATE_ONLINE $ACTIVATION_ID)
    SUCCESS=$(grep -z "OK\s{ \"key\"" <<< "$RESULT")

    if [ -z $SUCCESS ]; then
        echo "Online activation failed"
        return 255
    fi
    echo "Online activation successful"
    return 0
}

trap canceled SIGINT

COMMAND=
ACTIVATION_ID=

while [ $# -gt 0 ] ; do
    case "$1" in
        --help|-h)
            echo "Usage: $(basename $0) <command>"
            echo "Commands:"
            echo "   test_online"
            echo "   activate_online [options...]"
            echo "       --id|-i <activationID>"
            echo "   --help|-h"
            exit 1
            ;;
        test_online|activate_online)
            [ ! -z $COMMAND ] && echo "Command must be unique ($1)" && exit 1
            COMMAND="$1"
            ;;
        --id|-i)
            [ -z $COMMAND ] && echo "Command must be set before options ($1)" && exit 1
            [ $COMMAND != "activate_online" ] && echo "Invalid option for $COMMAND ($1)" && exit 1
            ACTIVATION_ID="$2"
            shift
            ;;
        *)  echo "Unrecognized params follow $*" >&2
            exit 1
            ;;
    esac
    shift
done

[ -z $COMMAND ] && echo "Command not set" && exit 1

case $COMMAND in
    "test_online")
        $COMMAND
        exit $?
        ;;
    "activate_online")
        $COMMAND $ACTIVATION_ID
        exit $?
        ;;
esac

echo "command '$COMMAND' not handled"
exit 1
