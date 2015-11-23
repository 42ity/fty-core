#!/bin/bash -x
#
#   Copyright (c) 2014 Eaton
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
#! \file    resetdb-bios-rc-demo.sh
#  \brief   Automates cloning of database contents from bios-rc-demo
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details This script automates cloning of database contents from bios-rc-demo
#           onto a box or container, and restarting of all services involved.
#           Used for internal testing.

REFHOST="bios-rc-demo"
REFFQDN="$REFHOST.roz53.lab.etn.com"
TMPFILE="/tmp/mysqldump-rc-demo.sql.gz"
DBNAME="box_utf8"

case "`hostname`" in
	$REFHOST|$REFHOST.*)
		echo "Running on the same host as the reference one - quitting!" >&2
		exit 1 ;;
	*)
		ping -c 4 -i 1 "$REFFQDN" || exit $?
		;;
esac

set -o pipefail

trap 'rm -f ${TMPFILE}' EXIT

if ! ssh "$REFFQDN" "mysqldump --databases ${DBNAME} | gzip" > ${TMPFILE} ; then
    echo "Could not get a dumpo of database ${DBNAME} from $REFFQDN into ${TMPFILE}" >&2
    rm -f ${TMPFILE}
    trap - EXIT
    exit 2
fi

echo "INFO: Got the database dump, starting destructive actions"

echo "Stopping services..."
systemctl stop bios-db-init malamute tntnet@bios.service mysql 'biostimer*.timer'
rm -f /var/lib/bios/agent-cm/biostimer-graphs-prefetch*.time

echo "Stopping mysql..."
systemctl start mysql 

echo "Dropping old database (if any)..."
mysql -u root -e "drop database ${DBNAME}"

echo "Importing database..."
gzip -cd < ${TMPFILE} | mysql || \
    { echo "FATAL: Oops, we killed old DB but could not import new one" >&2 ; exit 3; }
rm -f ${TMPFILE}
trap - EXIT

echo "Restarting services so they pick up dependencies well"
systemctl restart mysql bios-db-init malamute tntnet@bios.service 'biostimer*.timer'

sleep 5

echo "Restarting prefetchers..."
systemctl restart \
	biostimer-graphs-prefetch@15m::realpower.default.service \
	biostimer-graphs-prefetch@24h::realpower.default.service \
	biostimer-outage.service

echo "Status check:"
systemctl list-units -a '*bios*' '*tntnet*' '*malamute*' '*mysql*' | cat

