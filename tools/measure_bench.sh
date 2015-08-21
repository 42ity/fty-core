#!/bin/sh

#
# Copyright (C) 2015 Eaton
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


#! \file measure_bench.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \brief Not yet documented file

HOST="bios-rc-seed6"
MYSQL="mysql -h $HOST -P 3306 -u root"
RETRY="25"

if [ -z "`echo "select 'works';" | $MYSQL | grep works`" ]; then
    echo "Can't connect to database!"
    exit 1
fi

if [ "x$1" = "x-f" ]; then
    FORCE="yes"
    shift
else
    FORCE=""
fi

ENGINES="blackhole aria aria_u1 aria_u2 aria_idless aria_part aria_part_l aria_part_h innodb"

[ -z "$1" ] || ENGINES="$@"

for ENGINE in $ENGINES; do

echo "Testing engine '$ENGINE'"
export unix_ts=1436400000

if [ "`echo $ENGINE | grep _part`" ]; then
    REAL_ENGINE="`echo $ENGINE | sed 's|_part.*||'`"
    if [ "`echo $ENGINE | grep _part_h`" ]; then
        PART="PARTITION BY HASH(timestamp DIV 3600 + topic_id) PARTITIONS 24"
    else
        PART="PARTITION BY `echo $ENGINE | sed -n 's|.*_part_l|LINEAR|p'` KEY (timestamp,topic_id) PARTITIONS 24"
    fi
else
    REAL_ENGINE="$ENGINE"
    PART=""
fi
if [ -n "`echo "$ENGINE" | grep idless`" ]; then
    IDLESS="yes"
    REAL_ENGINE="`echo $REAL_ENGINE | sed 's|_idless.*||'`"
else
    IDLESS=""
fi
if [ -n "`echo "$ENGINE" | grep _u[12]$`" ]; then
    REAL_ENGINE="`echo $REAL_ENGINE | sed 's|_u[12]$||'`"
    if [ -n "`echo "$ENGINE" | grep _u1$`" ]; then
        UN=", CONSTRAINT un_tt UNIQUE(timestamp, topic_id)"
    else
        UN=", CONSTRAINT un_tt UNIQUE(topic_id, timestamp)"
    fi
else
    UN=""
fi

[ -z "$FORCE" ] || echo "DROP DATABASE $REAL_ENGINE;" | $MYSQL > /dev/null 2> /dev/null

if [ -z "`echo "show databases" | $MYSQL | grep "^bios_bench_$ENGINE\\\$"`" ]; then

$MYSQL << EOF
CREATE DATABASE IF NOT EXISTS bios_bench_$ENGINE character set utf8 collate utf8_general_ci;

USE bios_bench_$ENGINE;

CREATE TABLE t_bios_measurement_topic(
    id               INTEGER UNSIGNED  NOT NULL AUTO_INCREMENT,
    device_id        INTEGER UNSIGNED  NOT NULL DEFAULT '0',
    units            VARCHAR(10)       NOT NULL,
    topic            VARCHAR(255)      NOT NULL,
    PRIMARY KEY(id),

    INDEX(device_id,topic,units),
    UNIQUE INDEX \`UI_t_bios_measurement_topic\` (\`device_id\`, \`units\`, \`topic\`  ASC)

);

CREATE TABLE t_bios_measurement (
`[ -n "$IDLESS""$PART" ] || echo "    id            BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,"`
    timestamp     BIGINT              NOT NULL,
    value         INTEGER             NOT NULL,
    scale         SMALLINT            NOT NULL,
    topic_id      INTEGER UNSIGNED    NOT NULL,

`if [ -n "$PART""$IDLESS" ]; then
echo "    PRIMARY KEY(topic_id,timestamp),"
else
echo "    PRIMARY KEY(id),"
fi`
    INDEX(topic_id),
    INDEX(timestamp)
    $UN
`[ -n "$IDLESS""$PART" ] || echo "    ,INDEX(timestamp, topic_id)"`

`[ -n "$PART" ] || echo "   ,FOREIGN KEY(topic_id)
        REFERENCES t_bios_measurement_topic(id) ON DELETE CASCADE"`
) engine=$REAL_ENGINE $PART;

`for i in \`seq 1 100\`; do
echo "INSERT INTO t_bios_measurement_topic VALUES ($i,0,'x','noboby$i@nowhere');"
done`
EOF
echo "DB created"
start_ts="`expr $unix_ts - 10000`"
echo "Initial fillup"
for i in `seq $start_ts $unix_ts`; do
echo -n "INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES "
for j in `seq 2 100`; do
    echo -n "($i,0,0,$j),"
done
echo "($i,0,0,1);"
done | /usr/bin/time -f '%E' cat > /dev/null
sleep 5
fi

# Do benchmark
for number in 100 250 500 1000 5000; do
    echo
    echo "Inserting $number"
    end_ts="`expr $unix_ts + $number`"
    TOTAL=0
    TIME_T=0

    # Repeat ten times to get average
    for j in `seq 1 $RETRY`; do
        # Time whole test
        TIME_S="`date +%s`"

        # Generate some load
        for i in `seq 1 50`; do
            sleep 0.1
            echo "SELECT * FROM t_bios_measurement ORDER BY timestamp DESC LIMIT $i;" | $MYSQL bios_bench_$ENGINE > /dev/null
        done &

        # Do the inserts
        TIME="`for i in \`seq $unix_ts $end_ts\`; do
            echo "INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ($i,0,0,\`expr $i % 100\`);"
        done | /usr/bin/time -f '%E' $MYSQL bios_bench_$ENGINE 2>&1 | sed 's|[:\.]||g'`"
        echo " * run no. $j - $TIME" | sed 's|\([0-9][0-9]\)\([0-9][0-9]\)$|:\1.\2|'

        # Sum it up
        TOTAL="`expr $TOTAL + $TIME`"

        # Cleanup
        wait
        TIME_E="`date +%s`"
        TIME_T="`expr $TIME_T + $TIME_E - $TIME_S`"        
        echo "DELETE FROM t_bios_measurement WHERE timestamp >= $unix_ts;" | $MYSQL bios_bench_$ENGINE > /dev/null
        sleep 1;
    done
    echo "Averages: "
    ZEROS=""
    TOTAL="`expr $TOTAL / $RETRY`"
    TIME_T="`expr $TIME_T / $RETRY`"
    [ $TOTAL -gt 100    ] || ZEROS="0$ZEROS"
    [ $TOTAL -gt 1000   ] || ZEROS="0$ZEROS"
    [ $TOTAL -gt 10000  ] || ZEROS="0$ZEROS"
    [ $TOTAL -gt 100000 ] || ZEROS="0$ZEROS"
    echo "Insert $ZEROS$TOTAL" | sed 's|\([0-9][0-9]\)\([0-9][0-9]\)$|:\1.\2|'
    echo "Total $TIME_T"
done
echo

done
