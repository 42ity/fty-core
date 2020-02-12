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
# each object found inside it. See mvlndir() for wholesale relocation.
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

    if [[ ! -e "${OLD}" ]] || [[ ! -s "${OLD}" ]] || [[ -L "${OLD}" ]] ; then
        echo "Nothing to relocate: No '$OLD', or it is already a symlink" >&2
        return 0
    fi

    OLD_DIR=$(dirname "${OLD}") && [[ -n "${OLD_DIR}" ]] || return
    NEW_DIR=$(dirname "${NEW}") && [[ -n "${NEW_DIR}" ]] || return

    mkdir -p "${OLD_DIR}" || return
    mkdir -p "${NEW_DIR}" || return

    if [[ -d "${OLD}" ]]; then
        NUMOBJ="$(cd "${OLD}" && find . | wc -l)" || NUMOBJ=-1
        if [[ "$NUMOBJ" -le 1 ]]; then
            echo "Symlinking empty old directory: '$OLD' => '$NEW'" >&2
            if [[ -d "${NEW}" ]] ; then
                mv "${OLD}" "${OLD}.old-bios" && \
                ln -srf "${NEW}" "${OLD}" && \
                return $?
                # Keep the NEW access rights
            fi
            if [[ -e "${NEW}" ]] ; then
                echo "ERROR: '$NEW' already exists and is not a directory" >&2
                return 1
            fi
            mv "${OLD}" "${NEW}" || \
            mkdir -p "${NEW}" || return
            ln -srf "${NEW}" "${OLD}"
            # For empty NEW directory, fall through to optional resetting of access rights
        else
            # Create dirs, symlink files; chmod+chown later
            echo "Recursing into directory: '$OLD'" >&2
            ( cd "${OLD}" && find . | while read LINE ; do
                case "${LINE}" in
                    ""|.|./) ;;
                    *) (mvln "${OLD}/${LINE}" "${NEW}/${LINE}" "" "" ) || exit ;;
                esac
              done )
        fi
        RECURSE_FLAG="-R"
    else
        if [[ -f "${OLD}" ]]; then
            if [[ -e "${NEW}" ]]; then
                # If new setup has a file in an unpackaged directory
                # (so created by updated services), keep it in place.
                echo "Relocated as backup: '${OLD}' => '${NEW}.old-bios' because NEW file exists" >&2
                mv "${OLD}" "${NEW}.old-bios"
            else
                echo "Relocated: '${OLD}' => '${NEW}'" >&2
                mv "${OLD}" "${NEW}"
            fi
        fi

        # Make this symlink even if expected NEW file is currently missing
        echo "Symlink back: '${NEW}' => '${OLD}'" >&2
        ln -srf "${NEW}" "${OLD}"
    fi

    if [[ -n "${OWN}" ]] && [[ -e "${NEW}" ]] ; then
        chown $RECURSE_FLAG "${OWN}" "${NEW}"
    fi

    if [[ -n "${MOD}" ]] && [[ -e "${NEW}" ]] ; then
        chmod $RECURSE_FLAG "${MOD}" "${NEW}"
    fi
}

# Simply move a whole existing OLD directory to a NEW name, if NEW does not
# yet exist, and add a legacy symlink with the OLD name pointing to the NEW
# location. Generally it is safer (but slower) to mvln() recursively, with
# existence checks done for each object along the way.
mvlndir() {
    OLD="${1-}"
    NEW="${2-}"

    [[ -d "${NEW}" ]] && return 0

    if [[ ! -d "${OLD}" ]] || [[ -e "${NEW}" ]] ; then
        echo "Not relocating dir: '${OLD}' => '${NEW}' because LD does not exist or is not a dir" >&2
        return 1
    fi

    echo "Relocating dir: '${OLD}' => '${NEW}'" >&2

    NEW_DIR=$(dirname "${NEW}") && \
    [[ -n "${NEW_DIR}" ]] && \
    mkdir -p "${NEW_DIR}" && \
    mv "${OLD}" "${NEW}" && \
    ln -srf "${NEW}" "${OLD}"
}

# Handle certain config files
mvln /etc/agent-smtp/bios-agent-smtp.cfg /etc/fty-email/fty-email.cfg www-data: ""
if [ -s /etc/fty-email/fty-email.cfg.old-bios ]; then
    cp -pf /etc/fty-email/fty-email.cfg /etc/fty-email/fty-email.cfg.new-default
    cat /etc/fty-email/fty-email.cfg.old-bios | sed \
        -e 's|/var/lib/bios|/var/lib/fty|g' \
        -e 's|bios-agent-smtp|fty-email|' \
        -e 's|agent-smtp|fty-email|' \
    > /etc/fty-email/fty-email.cfg
fi


mvln /etc/agent-metric-store/bios-agent-ms.cfg /etc/fty-metric-store/fty-metric-store.cfg www-data: ""

mvln /etc/agent-nut/bios-agent-nut.cfg /etc/fty-nut/fty-nut.cfg www-data: ""

mvln /etc/default/bios.cfg /etc/default/fty.cfg www-data: ""

mvln /etc/default/bios /etc/default/fty www-data: ""

# Dirs with same content and access rights
mvlndir /var/lib/fty/nut /var/lib/fty/fty-nut

# 42ity renaming
mvln /etc/bios /etc/fty
mvln /etc/pam.d/bios /etc/pam.d/fty
mvln /usr/libexec/bios /usr/libexec/fty
mvln /var/lib/bios/license /var/lib/fty/fty-eula/license
mvln /var/lib/fty/license /var/lib/fty/fty-eula/license
chown www-data:sasl /var/lib/fty/fty-eula/license /var/lib/fty/fty-eula
chown 0:0 /var/lib/fty

# Note: currently we do want to keep tntnet@bios, which relies on old pathnames
#mvln /etc/tntnet/bios.d /etc/tntnet/fty.d

# Warning: order matters, somewhat
mvln /usr/share/bios /usr/share/fty
mvlndir /usr/share/bios/etc/default/bios /usr/share/fty/etc/default/fty

## Backward compatibility for new (renamed) paths
mvlndir /var/lib/bios/sql             /var/lib/fty/sql
mvln /var/lib/bios/bios-agent-cm      /var/lib/fty/fty-metric-compute
mvln /var/lib/bios/agent-outage       /var/lib/fty/fty-outage
mvln /var/lib/bios/agent-smtp         /var/lib/fty/fty-email
mvln /var/lib/bios/alert_agent        /var/lib/fty/fty-alert-engine
mvln /var/lib/bios/bios-agent-rt      /var/lib/fty/fty-metric-cache
mvln /var/lib/bios/composite-metrics  /var/lib/fty/fty-metric-composite
mvln /var/lib/bios/nut                /var/lib/fty/fty-nut
mvln /var/lib/bios/nut                /var/lib/fty/nut

# The /var/lib/fty/fty-sensor-env should now be created via tmpfiles
# But a legacy system may have an agent file of its own...
mvln /var/lib/bios/composite-metrics/agent_th  /var/lib/fty/fty-sensor-env/state

# alert list file must be converted, do it manually
if [[ -e /var/lib/bios/agent-alerts-list/state_file ]]; then
    mkdir -p /var/lib/fty/fty-alert-list
    chown bios:root /var/lib/fty/fty-alert-list
    /usr/bin/fty-alert-list-convert \
        state_file \
        /var/lib/bios/agent-alerts-list \
        /var/lib/fty/fty-alert-list
    chown bios:bios-infra /var/lib/fty/fty-alert-list/state_file || :
    rm -rf /var/lib/bios/agent-alerts-list
fi

# uptime file must be converted, do it manually
if [[ -e /var/lib/bios/uptime/state ]]; then
    mkdir -p /var/lib/fty/fty-kpi-power-uptime
    chown bios:root /var/lib/fty/fty-kpi-power-uptime
    /usr/bin/fty-kpi-power-uptime-convert \
        state \
        /var/lib/bios/uptime \
        /var/lib/fty/fty-kpi-power-uptime
    chown bios:bios-infra /var/lib/fty/fty-kpi-power-uptime/state || :
    rm -rf /var/lib/bios/uptime
fi

# Our web-server should be able to read these credentials,
# at least per current implementation of license_POST
# Note that on some systems these dirs are the same (so we
# reapply the settings twice to the same FS object), but
# on upgrades from legacy they can be different.
for CREDSQL_DIR in "/var/lib/fty/sql/mysql" "/var/lib/bios/sql/mysql" ; do
    mkdir -p "${CREDSQL_DIR}"
    chgrp www-data "${CREDSQL_DIR}"
    BIOS_DB_RW="$CREDSQL_DIR/bios-db-rw"
    if [[ -e "${BIOS_DB_RW}" ]] ; then
        chgrp www-data "${BIOS_DB_RW}"
        chmod g+r "${BIOS_DB_RW}"
    fi
done
