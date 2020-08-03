#!/bin/bash
#
# Copyright (C) 2016 - 2020 Eaton
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

#! \file test_averages_relative.sh
#  \author Karol Hrdina <karolhrdina@eaton.com>
#  \brief Not yet documented file
#  \details $1 - path to directory with mysql database files 


read -r -d '' OUT <<EOF
use box_utf8;

SELECT @topic_temperature := id FROM t_bios_measurement_topic
WHERE device_id=(SELECT id_discovered_device FROM t_bios_discovered_device WHERE name='AVG-SRV') AND
      units='C' AND
      topic='temperature.thermal_zone0@AVG-SRV';

SELECT @topic_power := id FROM t_bios_measurement_topic
WHERE device_id=(SELECT id_discovered_device FROM t_bios_discovered_device WHERE name='AVG-SRV') AND
      units='W' AND
      topic='realpower.default@AVG-SRV';

/* ************************************************************************ */
/* TEST CASE -1-                                                            */
/*      relative=7d step=24h type=arithmetic_mean source=@topic_temperature */
/* ************************************************************************ */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 04:00:00"), 10, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 15:00:00"), 20, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 16:00:00"), 30, 0, @topic_temperature);
/* timestamp: `date -ud '0 day ago 00:00:00' +%s`    avg_24h: 20 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '2 day ago' +%F` 01:00:00"), 100, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '2 day ago' +%F` 17:00:00"), 200, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '2 day ago' +%F` 23:00:00"), 300, 0, @topic_temperature);
/* timestamp: `date -ud '1 day ago 00:00:00' +%s`    avg_24h: 200 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '3 day ago' +%F` 04:00:00"), 20, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '3 day ago' +%F` 16:00:00"), 40, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '3 day ago' +%F` 17:00:00"), 60, 0, @topic_temperature);
/* timestamp: `date -ud '2 day ago 00:00:00' +%s`    avg_24h: 40 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '4 day ago' +%F` 02:00:00"), 200, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '4 day ago' +%F` 14:00:00"), 400, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '4 day ago' +%F` 16:00:00"), 600, 0, @topic_temperature);
/* timestamp: `date -ud '3 day ago 00:00:00' +%s`    avg_24h: 400 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '5 day ago' +%F` 03:00:00"), 40, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '5 day ago' +%F` 13:00:00"), 60, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '5 day ago' +%F` 15:00:00"), 80, 0, @topic_temperature);
/* timestamp: `date -ud '4 day ago 00:00:00' +%s`    avg_24h: 60 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '6 day ago' +%F` 01:00:00"), 400, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '6 day ago' +%F` 15:00:00"), 600, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '6 day ago' +%F` 16:00:00"), 800, 0, @topic_temperature);
/* timestamp: `date -ud '5 day ago 00:00:00' +%s`    avg_24h: 600 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '7 day ago' +%F` 01:00:00"), 60, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '7 day ago' +%F` 15:00:00"), 80, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '7 day ago' +%F` 17:00:00"), 100, 0, @topic_temperature);
/* timestamp: `date -ud '6 day ago 00:00:00' +%s`    avg_24h: 80 */
/* 7d window midnight aligned: 200 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '8 day ago' +%F` 06:00:00"), 600, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '8 day ago' +%F` 14:00:00"), 1000, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '8 day ago' +%F` 17:00:00"), 560, 0, @topic_temperature);
/* timestamp: `date -ud '7 day ago 00:00:00' +%s`    avg_24h: 720 */
/* 7d window midnight aligned: 300 */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '9 day ago' +%F` 06:00:00"), 600, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '9 day ago' +%F` 14:00:00"), 2000, 0, @topic_temperature);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '9 day ago' +%F` 17:00:00"), 100, 0, @topic_temperature);
/* timestamp: `date -ud '8 day ago 00:00:00' +%s`    acg_24: 900 */
/* 7d window midnight aligned: 400 */


/* ******************************************************************* */
/* TEST CASE -2-                                                       */
/*      relative=24h step=24h type=arithmetic_mean source=@topic_power */
/* ******************************************************************* */

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:00:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:05:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:10:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:15:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:20:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:25:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:30:00"), 10, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 01:35:00"), 10, 0, @topic_power);

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:20:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:25:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:30:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:35:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:40:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:45:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 02:50:00"), 30, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 03:55:00"), 30, 0, @topic_power);

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 21:05:00"), 80, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 21:15:00"), 80, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 21:20:00"), 80, 0, @topic_power);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (UNIX_TIMESTAMP("`date -ud '1 day ago' +%F` 21:25:00"), 80, 0, @topic_power);
/* timestamp: `date -ud '0 day ago 00:00:00' +%s`    avg_24: 32 */

EOF
echo "$OUT" > "$1"/test_averages_relative.sql


# TEST CASE -1- expected results
AVERAGES_RELATIVE_PATTERNS_1="{\"value\":20,\"timestamp\":`date -ud '0 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":200,\"timestamp\":`date -ud '1 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":40,\"timestamp\":`date -ud '2 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":400,\"timestamp\":`date -ud '3 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":60,\"timestamp\":`date -ud '4 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":600,\"timestamp\":`date -ud '5 day ago 00:00:00' +%s`}"
AVERAGES_RELATIVE_PATTERNS_1="$AVERAGES_RELATIVE_PATTERNS_1 {\"value\":80,\"timestamp\":`date -ud '6 day ago 00:00:00' +%s`}"
echo "$AVERAGES_RELATIVE_PATTERNS_1"

# TEST CASE -2- expected results
AVERAGES_RELATIVE_PATTERNS_2="{\"value\":32,\"timestamp\":`date -ud '0 day ago 00:00:00' +%s`}"
echo "$AVERAGES_RELATIVE_PATTERNS_2"
