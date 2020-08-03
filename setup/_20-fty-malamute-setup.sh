#!/bin/bash

#
#   Copyright (C) 2018 - 2020 Eaton.
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
#  \file    20-fty-malamute-setup.sh
#  \brief   Set up malamute service dependencies for 42ity bundling (rewrite original unit definition).
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>

# bashism to get errors from pipelines
set -o pipefail

die() {
    echo "FATAL: $*" >&2
    exit 1
}

# Dependencies can not be replaced, only appended -
# so we have to redefine the malamute unit.
# Also older systemd ignores the [Install] section in drop-ins, so set it here.
/bin/systemctl stop malamute || true
/bin/systemctl disable malamute ; /bin/systemctl mask malamute ; true


UNITFILE=/usr/lib/systemd/system/malamute.service
# Change the file in-place
if ! egrep -q '^PartOf=' "${UNITFILE}" ; then
    sed -e 's,^\(\[Unit\]\).*$,\1\nPartOf=\nBefore=\n,' \
        -i "${UNITFILE}" \
        || die "Failed to generate malamute for 42ity service"
fi

sed -e 's,^\(WantedBy\|RequiredBy\|PartOf\|Before\)=\(.*\)$,\1=bios-pre-eula.target,g' \
    -i "${UNITFILE}" \
    || die "Failed to generate malamute for 42ity service"

# Make sure malamute@ipm2.service gets started on this run that installed it
# (usually first boot, at all or after upgrade to this release)
mkdir -p /run/systemd/system/multi-user.target.wants/ \
    && ln -fsr "${UNITFILE}" \
        /run/systemd/system/multi-user.target.wants/malamute.service

/bin/systemctl daemon-reload
/bin/systemctl enable malamute
/bin/systemctl daemon-reload
