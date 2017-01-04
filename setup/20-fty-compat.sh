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
#

# Move file to new location and link it back
mvln () {
    OLD="${1}"
    NEW="${2}"

    OLD_DIR=$(dirname "${OLD}")
    NEW_DIR=$(dirname "${NEW_}")

    mkdir -p "${OLD_DIR}"
    mkdir -p "${NEW_DIR}"

    if [[ -e "${OLD}" ]]; then
        mv "${OLD}" "${NEW}"
        ln -sf "${NEW}" "${OLD}"
    fi
}

mvln /etc/agent-smtp/bios-agent-smtp.cfg /etc/fty-email/fty-email.cfg
chmod www-data: /etc/fty-email/fty-email.cfg
mvln /etc/agent-metric-store/bios-agent-ms.cfg /etc/fty-metric-store/fty-metric-store.cfg
chmod www-data: /etc/fty-metric-store/fty-metric-store.cfg
mvln /etc/agent-nut/bios-agent-nut.cfg /etc/fty-nut/fty-nut.cfg
chmod www-data: /etc/fty-nut/fty-nut.cfg
mvln /etc/default/bios.cfg /etc/default/fty.cfg
chmod www-data: /etc/default/fty.cfg
