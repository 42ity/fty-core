#!/bin/sh

#
#   Copyright (c) 2017 Eaton
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
#! \file    20-fty-compat.sh
#  \brief   Create compat symlinks
#  \author  Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

# Move an OLD file to NEW location (if it exists and is not a symlink)
# and link it back for legacy compatibility purposes; optionally
# set new ownership and access rights on the newly located file.
# If OLD filesystem object is a directory, recurse with mvln() for
# each object found inside it.
# Note: This assumes manipulations with files in deployment local
# data and config directories (not packaged) - so if some target
# filenames exist under FTY paths, we should not overwrite them with
# files from legacy BIOS paths.
mvln () {
    OLD="${1-}"
    NEW="${2-}"
    OWN="${3-}"
    MOD="${4-}"
    RECURSE_FLAG=""

    if [[ ! -s "${OLD}" ]] || [[ -L "${OLD}" ]] ; then
        # Nothing to relocate
        return 0
    fi

    OLD_DIR=$(dirname "${OLD}") && [[ -n "${OLD_DIR}" ]] || return
    NEW_DIR=$(dirname "${NEW}") && [[ -n "${NEW_DIR}" ]] || return

    mkdir -p "${OLD_DIR}" || return
    mkdir -p "${NEW_DIR}" || return

    if [[ -d "${OLD}" ]]; then
        # Create dirs, symlink files; chmod+chown later
        ( cd "${OLD}" && find . | while read LINE ; do
            mvln "${OLD}/${LINE}" "${NEW}/${LINE}" "" "" || exit
          done )
        RECURSE_FLAG="-R"
    else
        if [[ -f "${OLD}" ]]; then
            if [[ -e "${NEW}" ]]; then
                # If new setup has a file in an unpackaged directory
                # (so created by updated services), keep it in place.
                mv "${OLD}" "${NEW}.old-bios"
            else
                mv "${OLD}" "${NEW}"
            fi
        fi

        # Make this symlink even if expected NEW file is currently missing
        ln -srf "${NEW}" "${OLD}"
    fi

    if [[ -n "${OWN}" ]] && [[ -e "${NEW}" ]] ; then
        chown $RECURSE_FLAG "${OWN}" "${NEW}"
    fi

    if [[ -n "${MOD}" ]] && [[ -e "${NEW}" ]] ; then
        chmod $RECURSE_FLAG "${MOD}" "${NEW}"
    fi
}

# Handle certain config files
# FIXME: Ownership by "www-data" seems wrong for many of these, unless
# we deliberately want web-server to edit these files (may be true)?
mvln /etc/agent-smtp/bios-agent-smtp.cfg /etc/fty-email/fty-email.cfg www-data: ""

mvln /etc/agent-metric-store/bios-agent-ms.cfg /etc/fty-metric-store/fty-metric-store.cfg www-data: ""

mvln /etc/agent-nut/bios-agent-nut.cfg /etc/fty-nut/fty-nut.cfg www-data: ""

mvln /etc/default/bios.cfg /etc/default/fty.cfg www-data: ""

mvln /etc/default/bios /etc/default/fty www-data: ""

# Dirs with same content and access rights
mvln /var/lib/fty/nut /var/lib/fty/fty-nut
