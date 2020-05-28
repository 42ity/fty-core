#!/bin/bash

#
# Copyright (C) 2015 - 2020 Eaton
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


#! \file   element_to_topic.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \brief  Not yet documented file

usage() {
        echo "Usage:
$0 {ELEMENT_ID} {SOURCE}"
}

[ $# != 2 ] && \
	echo "ERROR: Wrong amount of arguments!" >&2 && \
	usage >&2 && exit 1
[ -z "$1" -o -z "$2" ] && \
	echo "ERROR: One of arguments is empty!" >&2 && \
	usage >&2 && exit 1

ELEMENT_ID="$1"
SOURCE="$2"

# TODO: Later this may be an UUID - in that future, change accordingly
if ! [ "$ELEMENT_ID" -ge 0 ] ; then
	echo "ERROR: ELEMENT_ID must be a non-negative number (uint64)!" >&2 && \
	usage >&2 && exit 1
fi

mysql -u root -D box_utf8 -N -e "    
SELECT @id := id FROM t_bios_measurement_topic,
    (
    SELECT @device_id := a.id_discovered_device, @name := name
    FROM
        t_bios_discovered_device AS a LEFT JOIN t_bios_monitor_asset_relation AS b
        ON a.id_discovered_device = b.id_discovered_device
    WHERE
        id_asset_element = '${1}'
    ) AS c
WHERE
    device_id = @device_id AND
    topic = CONCAT('${2}@', @name)"

