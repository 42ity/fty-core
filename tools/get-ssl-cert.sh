#!/bin/bash

#
# Copyright (C) 2015 - 2021 Eaton
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
#! \file   get-ssl-cert.sh
#  \author Mauro Guerrera <MauroGuerrera@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \brief  Use a third-party "certcmd" to provide or update HTTPS certs

# Note: bashism, so we know piped commands failed
set -o pipefail

PEM_KEY="/etc/tntnet/bios.key"
PEM_CRT="/etc/tntnet/bios.crt"
PEM_FINAL_CERT="/etc/tntnet/bios.pem"

command -v certcmd || { echo "FATAL: certcmd not in PATH!" >&2; exit 1; }

for F in "$PEM_FINAL_CERT" "$PEM_KEY" "$PEM_CRT" ; do
    D="`dirname "$F"`"
    if [ ! -d "$D/" ]; then
        echo "FATAL: Directory '$D' to hold the PEM files does not exist!" >&2
        exit 1
    fi
done

discardEOLs() {
    ### discard multiple new lines at the end of the stream
    sed -E ':a;N;$!ba;s/[\n]+$//g'
}

KEY="$(certcmd https server getkey | discardEOLs)" && [ -n "$KEY" ] \
|| { echo "FATAL: Could not generate or fetch KEY data!" >&2 ; exit 1; }
CRT="$(certcmd https server getcert | discardEOLs)" && [ -n "$CRT" ] \
|| { echo "FATAL: Could not generate or fetch CRT data!" >&2 ; exit 1; }

UPDATE_CERT=no
if [ -f "$PEM_KEY" ]; then
    DIFF="$(diff -q <(echo "$KEY") <(discardEOLs < "$PEM_KEY") | grep differ)"
    if [ -n "$DIFF" ]; then
        UPDATE_CERT=yes
    else
        echo "INFO: KEY file is same as before, still valid" >&2
    fi
else
    UPDATE_CERT=yes
fi

if [ -f "$PEM_CRT" ]; then
    DIFF="$(diff -q <(echo "$CRT") <(discardEOLs < "$PEM_CRT") | grep differ)"
    if [ -n "$DIFF" ]; then
        UPDATE_CERT=yes
    else
        echo "INFO: CRT file is same as before, still valid" >&2
    fi
else
    UPDATE_CERT=yes
fi

if [ "$UPDATE_CERT" = yes ]; then
    echo "INFO: got new KEY and/or CRT contents, updating files for SSL-enabled servers" >&2

    echo "$KEY" > "$PEM_KEY" \
    && [ -s "$PEM_KEY" ] || exit

    echo "$CRT" > "$PEM_CRT" \
    && [ -s "$PEM_CRT" ] || exit

    cat "$PEM_KEY" "$PEM_CRT" > "$PEM_FINAL_CERT" \
    && [ -s "$PEM_FINAL_CERT" ] || exit
else
    echo "INFO: no new KEY nor CRT contents, nothing to do here now" >&2
fi

exit 0
