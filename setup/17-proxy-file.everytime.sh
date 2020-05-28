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
#! \file    17-proxy-file.everytime.sh
#  \brief   Make sure a properly owned file for HTTP(S) proxy setup exists
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

# Specific envvars for systemd services and maybe shell: the HTTP(S) proxy
# to connect to the licensing server, new image downloads, etc. from agents.
PROXY_FILE="/etc/default/fty-proxy"
PROXY_FILE_PROFILE="/etc/profile.d/fty-proxy.sh"
if test ! -s "$PROXY_FILE" ; then
    touch "$PROXY_FILE"
fi
# Not in the "if" above so we can change it in later revisions if needed
chown www-data:bios-admin "$PROXY_FILE"
chmod 664 "$PROXY_FILE"

if test ! -s "$PROXY_FILE_PROFILE" ; then
    rm -f "$PROXY_FILE_PROFILE"
    ln -fsr "$PROXY_FILE" "$PROXY_FILE_PROFILE"
fi

echo "`date -u`: Ensured usable file for HTTP(S) proxy setup:"
ls -la "$PROXY_FILE" "$PROXY_FILE_PROFILE"
