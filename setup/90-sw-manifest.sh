#!/bin/sh

#
#   Copyright (c) 2018 - 2020 Eaton
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
#! \file    90-sw-manifest.sh
#  \brief   Make sure /mnt/nand/rootfs/manifest exists and is owned
#           by proper account names even if name-to-number mapping has
#           changed in the user 
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Arnaud Quette <ArnaudQuette@Eaton.com>

SW_USER="bios"
SW_GROUP="www-data"
SW_UID="`getent passwd $SW_USER | cut -d':' -f3`"
SW_GID="`getent group $SW_GROUP | cut -d':' -f3`"

SW_MANIFEST_NAME="/mnt/nand/rootfs/manifest"
SW_MANIFEST_MODE="775"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

# Check that a directory $1 exists and
# * has owner GID $2 and group GID $3
# * has mode $4
# Returns
# 0 upon success
# 1 if directory does not exist
# 2 if directory exists but is not owned by UID $2 & GID $3
# 3 if directory exists and is owned by $2:$3, but has not mode $4
check_directory() {
    # Test directory existence
    [ -d "$1" ] || return 1

    # Test directory ownership
    DIRPATH="`dirname $1`/" # Beware of the needed ending slash!
    DIRNAME="`basename $1`"
    DIR_OWNED="`find "$DIRPATH" -name "$DIRNAME" -uid "$2" -gid "$3"`"
    [ -z "$DIR_OWNED" ] && return 2

    # Test directory mode
    DIR_MODE="`stat -c "%a" "$1"`"
    [ "$DIR_MODE" = "$4" ] || return 3

    return 0
}

# Sanity check
[ -z "$SW_UID" ] \
&& die "The '${SW_USER}' user account is not defined in this system"

[ -z "$SW_GID" ] \
&& die "The '${SW_GROUP}' group account is not defined in this system"

# Check if actions are needed or not
check_directory "$SW_MANIFEST_NAME" "$SW_UID" "$SW_GID" "$SW_MANIFEST_MODE"
RES=$?

# Process result
case $RES in
    1)
        mkdir -p "$SW_MANIFEST_NAME" || RES=$?
        chown -R "$SW_USER" "$SW_MANIFEST_NAME" || RES=$?
        chgrp -R "$SW_GROUP" "$SW_MANIFEST_NAME" || RES=$?
        chmod -R "$SW_MANIFEST_MODE" "$SW_MANIFEST_NAME" || RES=$?
        ;;
    2)
        chown -R "$SW_USER" "$SW_MANIFEST_NAME" || RES=$?
        chgrp -R "$SW_GROUP" "$SW_MANIFEST_NAME" || RES=$?
        chmod -R "$SW_MANIFEST_MODE" "$SW_MANIFEST_NAME" || RES=$?
        ;;
    3)
        chmod -R "$SW_MANIFEST_MODE" "$SW_MANIFEST_NAME" || RES=$?
        ;;
    0)
        ;;
esac

# Final check
check_directory "$SW_MANIFEST_NAME" "$SW_UID" "$SW_GID" "$SW_MANIFEST_MODE"
RES=$?

exit $RES
