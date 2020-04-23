#!/bin/sh

#
#   Copyright (c) 2017 - 2020 Eaton
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
#! \file    15-dpkg-database.everytime.sh
#  \brief   Detect if the OS image on HW appliance was changed, and migrate
#           the Debian-based OS packaging database information to rebase
#           the appliance from older release to newer one.
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

#  NOTES:
# Phase1) Replace the running-system info with data from OS image in case
#         of discrepancies. This is rude but efficient.
# Phase2) Find a nice way to merge the differences between last and current
#         OS images packaged constituents with the current system. If there
#         were no packaged customizations on appliance, this is like Phase1.
# Currently this script leaves around copies of the packaging database that
# it can inspect and compare. In practice it needs roughly one generation
# back, so should later be changed to remove or compress older ones, which
# may still be of interest for debugging or field support.

[ -n "${RO_ROOT-}" ] || RO_ROOT="/mnt/root-ro"
[ -n "${RW_ROOT-}" ] || RW_ROOT=""
if [ -z "${RW_ROOT-}" ] && [ -n "${ALTROOT-}" ] ; then
    RW_ROOT="${ALTROOT-}"
fi
[ -n "${DPKG_DIR-}" ] || DPKG_DIR="/var/lib/dpkg"
[ -n "${DPKG_INFO_DIR-}" ] || DPKG_INFO_DIR="${DPKG_DIR}/info"
[ -n "${DPKG_STATE-}" ] || DPKG_STATE="${DPKG_DIR}/status"
# We do not care for other dpkg databases yet, but this one
# might be interesting for Phase2 completeness. However, in
# OS images it is empty, so there is nothing to migrate...
#[ -n "${DPKG_AVAIL-}" ] || DPKG_AVAIL="${DPKG_DIR}/available"

LC_ALL=C
LANG=C
TZ=UTC
export LC_ALL LANG TZ

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

### Some cleanup and sanity checks on input variables
### No trailing slashes on root dir values
RO_ROOT="`echo "$RO_ROOT" | sed 's,/*$,,g'`"
RW_ROOT="`echo "$RW_ROOT" | sed 's,/*$,,g'`"

for D in "$DPKG_DIR" "$DPKG_INFO_DIR" "$DPKG_STATE" ; do
    case "$D" in
        /*) ;; # OK
        *)  die "Pathname must be complete, starting from root. Invalid value detected: '$D'" ;;
    esac
done

### RO root must be not system root, and be present
[ -n "${RO_ROOT}" ] && [ -d "${RO_ROOT}" ] \
|| skip "This script does not apply on this OS or HW: no read-only root filesystem to inspect at '${RO_ROOT}'"

### RW root (live) may be system root, or a valid alternate root
### (e.g. a chroot or container env as seen the from host env)
if [ -n "${RW_ROOT}" ] ; then
    # Note: This prefix may be empty in paths below,
    # but otherwise it must be a valid directory...
    # TODO: Should this die() as misconfig or just skip()?..
    [ -d "${RW_ROOT}" ] \
    || die "This script does not apply on this OS or HW: no alternate read-write root filesystem to manipulate at '${RW_ROOT}'"
fi

### Expected dirs and files exist
[ -d "${RO_ROOT}${DPKG_DIR}" ] && [ -d "${RW_ROOT}${DPKG_DIR}" ] \
&& [ -d "${RO_ROOT}${DPKG_INFO_DIR}" ] && [ -d "${RW_ROOT}${DPKG_INFO_DIR}" ] \
&& [ -s "${RO_ROOT}${DPKG_STATE}" ] && [ -s "${RW_ROOT}${DPKG_STATE}" ] \
|| skip "This script does not apply on this OS: no debian packaging database in live and/or or read-only root filesystem"

### Expected files can be manipulated
[ -r "${RO_ROOT}${DPKG_STATE}" ] && [ -r "${RW_ROOT}${DPKG_STATE}" ] && [ -w "${RW_ROOT}${DPKG_STATE}" ] \
&& [ -w "${RW_ROOT}${DPKG_DIR}" ] && [ -w "${RW_ROOT}${DPKG_DIR}/.." ] \
|| skip "This script does not apply on this OS: insufficient access to manipulate the debian packaging database"

# First make sure we save the copy of current OS image's data, if it is missing
# or needs to be updated. Then check the current OS if it was customized.
# If done the other way around, we have no "lastroot" to compare against
# after upgrades.
# For Phase2: Also take care to not remove this data until we merge databases.

TIMESTAMP="$(date -u '+%Y%m%dT%H%M%SZ')" || TIMESTAMP="$(date -u '+%s')" || TIMESTAMP=""
[ -n "$TIMESTAMP" ] || TIMESTAMP="$$"
TIMESTAMP_START="${TIMESTAMP}"

while [ -d "${RW_ROOT}${DPKG_DIR}.old-${TIMESTAMP}" ] || [ -d "${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}" ] ; do
    TIMESTAMP="${TIMESTAMP_START}-$(head -c 16 /dev/random | base64 | sed 's,[^A-Za-z0-9]*,,g')"
done

if [ -d "${RW_ROOT}${DPKG_DIR}.lastroot" ]; then
    diff -qr "${RO_ROOT}${DPKG_DIR}" "${RW_ROOT}${DPKG_DIR}.lastroot" >/dev/null 2>&1 \
    && skip "Packaging database on this system is already up to date: the read-only OS image did not change since last boot"

    mv -f "${RW_ROOT}${DPKG_DIR}.lastroot" "${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}" \
    && sync \
    || die "Could not stash away a copy of the previous read-only OS image packaging into ${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}"
fi

cp -prf "${RO_ROOT}${DPKG_DIR}" "${RW_ROOT}${DPKG_DIR}.lastroot" \
&& sync \
|| die "Could not stash away a copy of the current read-only OS image packaging into ${RW_ROOT}${DPKG_DIR}.lastroot"

diff -qr "${RO_ROOT}${DPKG_DIR}" "${RW_ROOT}${DPKG_DIR}" >/dev/null 2>&1 \
&& skip "Packaging database on this system is already up to date: same as on the read-only OS image"

# Phase2: TODO: instead of just replacing data as fast-tracked above, merge
# the changes between old and new packaging states into the live system.
# Note that we've already ruled out the case of unchanged OS image and locally
# modified packaging database, so the remaining case is really merging stuff.
ACTION="replace"
if [ -d "${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}" ]; then
    if diff -qr "${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}" "${RW_ROOT}${DPKG_DIR}" >/dev/null 2>&1 ; then
        echo "NOTICE: Packaging database on this system was not changed while booted with"
        echo "previous read-only OS images so we can just replace it."
    else
        # ...merge... else goto Phase1
        ACTION="merge"
        echo "=========="
        echo "WARNING: $0 detected that the OS image has changed since last boot, and that"
        echo "some packaging changes were applied on the live system since its deployment."
        echo "This use-case is currently NOT SUPPORTED, so the packaging database will be"
        echo "effectively replaced with the contents delivered by a new read-only OS image!"
        echo "=========="
    fi
fi

# Phase1: rude implementation
case "$ACTION" in
    replace|merge)
        if [ "$ACTION" = merge ]; then
            diff -Naur "${RW_ROOT}${DPKG_DIR}.lastroot-${TIMESTAMP}" "${RW_ROOT}${DPKG_DIR}" \
                > "/tmp/.dpkg-customizations-${TIMESTAMP}.diff" \
            && mv -f "/tmp/.dpkg-customizations-${TIMESTAMP}.diff" "${RW_ROOT}${DPKG_DIR}" \
            && sync \
            && echo "Differences in metadata have been saved into '${RW_ROOT}${DPKG_DIR}.old-${TIMESTAMP}/.dpkg-customizations-${TIMESTAMP}.diff'"
        fi

        echo "APPLY: Replacing whole ${RW_ROOT}${DPKG_DIR} with contents of ${RO_ROOT}${DPKG_DIR} (old state will stay in ${RW_ROOT}${DPKG_DIR}.old-${TIMESTAMP})"
        cp -prf "${RO_ROOT}${DPKG_DIR}" "${RW_ROOT}${DPKG_DIR}.new-${TIMESTAMP}" \
        && sync \
        && mv -f "${RW_ROOT}${DPKG_DIR}" "${RW_ROOT}${DPKG_DIR}.old-${TIMESTAMP}" \
        && mv -f "${RW_ROOT}${DPKG_DIR}.new-${TIMESTAMP}" "${RW_ROOT}${DPKG_DIR}" \
        && sync \
        && echo "SUCCEEDED replacing the live root database from updated read-only root" \
        || die "FAILED to replace the live root database from updated read-only root"
        ;;
    *) die "ACTION '$ACTION' is not implemented nor supported (yet?)" ;;
esac

### Final sanity check
[ -d "${RW_ROOT}${DPKG_DIR}" ] && [ -s "${RW_ROOT}${DPKG_STATE}" ] && [ -d "${RW_ROOT}${DPKG_INFO_DIR}" ] \
|| die "Debian packaging metadata is not accessible as exected in the RW_ROOT, this is likely a scripting error in $0"
