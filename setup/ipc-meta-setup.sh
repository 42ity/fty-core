#!/bin/sh

#
#   Copyright (c) 2014-2017 Eaton
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
#! \file    ipc-meta-setup.sh
#  \brief   Script to manage modular initial self-configuration of an IPC
#  \author  Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#

# Ensure consistent alphabetic sorting of script names, etc.
LC_ALL=C
LANG=C
TZ=UTC
export LC_ALL LANG TZ

BASEDIR="$(dirname $(readlink -f ${0}))"
SETUPDIR=/var/lib/fty/ipc-meta-setup/

die () {
    echo "FATAL: " "${@}" >&2
    exit 1
}

[ -n "${BASEDIR}" ] && [ -d "${BASEDIR}" ] || die "BASEDIR '${BASEDIR}' is not available"

mkdir -p "${SETUPDIR}"

# Make sure scripts do not leave occasional traces in unexpected places
cd /tmp || die "No /tmp!"

ls -1 "${BASEDIR}"/[0-9]*.sh | sort | while read SCRIPT; do

    # We generally run scripts once, to set up a newly deployed system,
    # or to update something after an upgrade to the new feature level.
    # The component scripts are expected to deliver one set of changes
    # and never change functionality across releases (similar to SQL
    # schema update bit by bit), so they are marked for not re-running
    # later.
    SCRIPT_NAME="$(basename "${SCRIPT}")"
    if [ -f "${SETUPDIR}/${SCRIPT_NAME}.done" ]; then
        echo "SKIP: ${SCRIPT_NAME} has already succeeded before"
    else
        echo "APPLY: running ${SCRIPT_NAME}..."
        ${SCRIPT} || die "${SCRIPT}"
        touch "${SETUPDIR}/${SCRIPT_NAME}.done"
    fi

done
