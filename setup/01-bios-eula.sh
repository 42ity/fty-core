#!/bin/sh

#
#   Copyright (c) 2019 Eaton
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
#! \file    01-bios-eula.sh
#  \brief   Make sure the license file created after EULA acceptance is in right location
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

EULA_USER="www-data"
EULA_GROUP="sasl"
EULA_DIR="/var/lib/fty/fty-eula"
EULA_FILE="${EULA_DIR}/license"
OLD_FILE="/var/lib/fty/license"

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

# Is the setup correct already? (Not an upgrade of an older deployment)
if [ -d "${EULA_DIR}/" ] ; then
    if [ -L "${OLD_FILE}" ] || [ ! -s "${OLD_FILE}" ] ; then
        return 0
    fi
fi

# The essense of this change is that directory which contains systemd
# tmpfiles for our ecosystem accounts must be root-owned. And web-server
# previously wrote into it directly. So we fix this bit before others.
chown 0:0 /var/lib/fty
chmod ugo+x /var/lib/fty # Make sure everyone can at least traverse it to find the EULA file by known name

getent passwd "${EULA_USER}" >/dev/null \
|| die "The '${EULA_USER}' user account is not defined in this system"

getent group "${EULA_GROUP}" >/dev/null \
|| die "The '${EULA_GROUP}' group account is not defined in this system"

mkdir -p "${EULA_DIR}" || die "Could not create EULA_DIR='${EULA_DIR}'"
chown "${EULA_USER}:${EULA_GROUP}" "${EULA_DIR}" || die "Could not chown '${EULA_DIR}' to '${EULA_USER}:${EULA_GROUP}'"
chmod 755 "${EULA_DIR}" || die "Could not chmod '${EULA_DIR}' to 755"

# Let the users of old name still use it until updated
if [ -e "${OLD_FILE}" ] && [ ! -L "${OLD_FILE}" ] ; then
    if [ -e "${EULA_FILE}" ] && [ ! -L "${EULA_FILE}" ] ; then
        die "Both OLD_FILE='${OLD_FILE}' and EULA_FILE='${EULA_FILE}' exist; this should not happen"
    fi
    mv -f "${OLD_FILE}" "${EULA_FILE}" || die "Could not relocate OLD_FILE='${OLD_FILE}' into EULA_FILE='${EULA_FILE}'"
fi
ln -fsr "${EULA_FILE}" "${OLD_FILE}" || die "Could not leave a legacy symlink OLD_FILE='${OLD_FILE}' to EULA_FILE='${EULA_FILE}'"
