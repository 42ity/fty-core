#!/bin/bash -x

### This script automates cloning of database contents from bios-rc-demo
### onto a box or container, and restarting of all services involved.
### Used for internal testing.
### Author: Jim Klimov <EvgenyKlimov@eaton.com>

REFHOST="bios-rc-demo"
REFFQDN="$REFHOST.roz.lab.etn.com"
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

systemctl stop bios-db-init malamute tntnet@bios.service mysql
rm -f /var/lib/bios/agent-cm/biostimer-graphs-prefetch*.time

systemctl start mysql 
mysql -u root -e "drop database ${DBNAME}"
gzip -cd < ${TMPFILE} | mysql || \
    { echo "FATAL: Oops, we killed old DB but could not import new one" >&2 ; exit 3; }
rm -f ${TMPFILE}
trap - EXIT

systemctl restart mysql bios-db-init malamute tntnet@bios.service

sleep 5

systemctl list-units -a '*bios*' '*tntnet*' '*malamute*' '*mysql*'

systemctl restart \
	biostimer-graphs-prefetch@15m::realpower.default.service \
	biostimer-graphs-prefetch@24h::realpower.default.service \
	biostimer-outage.service

