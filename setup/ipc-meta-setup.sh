#!/bin/sh

#
#   Copyright (c) 2014 - 2020 Eaton
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
# TODO: Use configure.ac templated variables?
SETUPDIR=/var/lib/fty/ipc-meta-setup/

die () {
    echo "FATAL: " "`date -u`: " "${@}" >&2
    exit 1
}

[ -n "${BASEDIR}" ] && [ -d "${BASEDIR}" ] || die "BASEDIR '${BASEDIR}' is not available"

mkdir -p "${SETUPDIR}"

# Make sure scripts do not leave occasional traces in unexpected places
cd /tmp || die "No /tmp!"

# Log the reason of untimely demise for typical causes (e.g. systemd timeout)...
trap 'META_RES=$? ; echo "$0: Aborting due to SIGTERM at `date -u`" >&2 ; exit $META_RES;' 15
trap 'META_RES=$? ; echo "$0: Aborting due to SIGINT  at `date -u`" >&2 ; exit $META_RES;'  2
trap 'META_RES=$? ; echo "$0: Aborting due to SIGQUIT at `date -u`" >&2 ; exit $META_RES;' 3
trap 'META_RES=$? ; echo "$0: Aborting due to SIGABRT at `date -u`" >&2 ; exit $META_RES;' 6

echo "STARTING: $0 for scriptlets under ${BASEDIR}, at `date -u`..."
ls -1 "${BASEDIR}"/[0-9]*.sh | sort | while read SCRIPT; do

    # We generally run scripts once, to set up a newly deployed system,
    # or to update something after an upgrade to the new feature level.
    # The component scripts are expected to deliver one set of changes
    # and never change functionality across releases (similar to SQL
    # schema update bit by bit), so they are marked for not re-running
    # later. Aside from that we also have some scripts that we do run
    # during every boot and they evaluate if they should act this time
    # or quickly abort without error if there is nothing to do.
    SCRIPT_NAME="$(basename "${SCRIPT}")"
    EVERY_TIME="no"
    case "$SCRIPT_NAME" in
        *.everytime.sh) EVERY_TIME="yes" ;;
    esac

    if [ -f "${SETUPDIR}/${SCRIPT_NAME}.done" ]; then
        ECHO_LABEL="SKIP"
        [ "$EVERY_TIME" = "yes" ] && ECHO_LABEL="NOTE"
        echo "${ECHO_LABEL}: ${SCRIPT_NAME} has already succeeded before"
        [ "$EVERY_TIME" = "yes" ] || continue
    fi

    echo "APPLY: running ${SCRIPT_NAME} at `date -u`..."
    if [ -x "${SCRIPT}" ]; then
        "${SCRIPT}"
    else
        echo "WARNING: ${SCRIPT_NAME} is not executable by itself, sub-shelling to run it. Its original shebang is: `head -1 "${SCRIPT}" | egrep '^\#\!\/'`" >&2
        ( . "${SCRIPT}" )
    fi || die "${SCRIPT} failed with exit-code $?, not proceeding with other scripts"
    touch "${SETUPDIR}/${SCRIPT_NAME}.done" && \
    echo "APPLIED: successfully ran ${SCRIPT_NAME}, finished at `date -u`..."

done || die "Something in $0 failed, aborting"

echo "COMPLETED: successfully ran $0, finished at `date -u`..."
