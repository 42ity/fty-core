#/!bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: sets up the sandbox and runs the tests of REST API for
# the $BIOS project.

if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

[ -z "$BUILDSUBDIR" -o ! -d "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
[ ! -x "$BUILDSUBDIR/config.status" ] && BUILDSUBDIR="$PWD"
if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "CI-ERROR: Cannot find $BUILDSUBDIR/config.status, did you run configure?"
    echo "CI-ERROR: Search path checked: $CHECKOUTDIR, $PWD"
    exit 1
fi

echo "CI-INFO: Using BUILDSUBDIR='$BUILDSUBDIR' to run the REST API webserver"

[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"
[ -z "$BIOS_PORT" ] && BIOS_PORT="8000"

DB_LOADDIR="$CHECKOUTDIR/tools"
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_TOPOP="power_topology.sql"
DB_TOPOL="location_topology.sql"

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

# Simple check for whether sudo is needed to restart saslauthd
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

usage(){
    echo "Usage: $(basename $0) [options...] [test_name...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
}

while [ $# -gt 0 ] ; do
    case "$1" in
        --user|-u)
            BIOS_USER="$2"
            shift
            ;;
        --passwd|-p)
            BIOS_PASSWD="$2"
            shift
            ;;
	--help|-h)
            usage
            exit 1
            ;;
        *)  # Assume that list of test names follows
    	    # (positive or negative, see test_web.sh)
            break
    	    ;;
    esac
    shift
done

set -u
set -e

test_web_port() {
    netstat -tan | grep -w "${BIOS_PORT}" | egrep 'LISTEN' >/dev/null
}

test_web_process() {
    [ -z "$MAKEPID" ] && return 0

    if [ ! -d /proc/$MAKEPID ]; then
	echo "CI-ERROR: Web-server process seems to have died!" >&2
	# Ensure it is dead though, since we abort the tests now
	kill $MAKEPID >/dev/null 2>&1
	wait $MAKEPID >/dev/null 2>&1
	return
    fi
    return 0
}

wait_for_web() {
    for a in $(seq 60) ; do
        sleep 5
	test_web_process || exit
        if ( test_web_port ) ; then
            return 0
        fi
    done
    echo "CI-ERROR: Port ${BIOS_PORT} still not in LISTEN state" >&2
    return 1
}


# prepare environment
  cd $CHECKOUTDIR || { echo "FATAL: Unusable CHECKOUTDIR='$CHECKOUTDIR'" >&2; exit 1; }

  # might have some mess
  killall tntnet 2>/dev/null || true
  sleep 1
  test_web_port && echo "CI-ERROR: Port ${BIOS_PORT} is in LISTEN state when it should be free" >&2 && exit 1

  # make sure sasl is running
  if ! $RUNAS systemctl --quiet is-active saslauthd; then
    $RUNAS systemctl start saslauthd || \
      [ x"$RUNAS" = x ] || \
      echo "CI-WARNING: Could not restart saslauthd, make sure SASL and SUDO are installed and /etc/sudoers.d/bios_01_citest is set up per INSTALL docs" >&2
  fi
  # check SASL is working
  echo "CI-INFO: Checking local SASL Auth Daemon"
  testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios && \
    echo "CI-INFO: saslauthd is responsive and configured well!" || \
    echo "CI-ERROR: saslauthd is NOT responsive or not configured!" >&2

# do the webserver
  # make clean
  LC_ALL=C
  LANG=C
  export BIOS_USER BIOS_PASSWD LC_ALL LANG
  echo "CI-INFO: Ensure files for web-test exist and are up-to-date..."
  make -C "$BUILDSUBDIR" V=0 web-test-deps || exit
  echo "CI-INFO: Spawn the web-server in the background..."
  make -C "$BUILDSUBDIR" web-test &
  MAKEPID=$!
  echo "CI-INFO: Wait for web-server to begin responding..."
  wait_for_web && \
    echo "CI-INFO: Web-server is responsive!" || \
    echo "CI-ERROR: Web-server is NOT responsive!" >&2
  echo "CI-INFO: Waiting for webserver process $MAKEPID to settle after startup..."
  sleep 5
  test_web_process || exit

test_web() {
    echo "============================================================"
    /bin/bash tests/CI/test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD" "$@"
    RESULT=$?
    echo "============================================================"
    return $RESULT
}

loaddb_file() {
    mysql -u root < "$1" > /dev/null
}

loaddb_default() {
    echo "--------------- reset db: default ----------------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_DATA"
}

test_web_default() {
    loaddb_default && \
    test_web "$@"
}

test_web_topo_p() {
    echo "----------- reset db: topology : power -----------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_TOPOP" && \
    test_web "$@"
}

test_web_topo_l() {
    echo "---------- reset db: topology : location ---------"
    loaddb_file "$DB_LOADDIR/$DB_BASE" && \
    loaddb_file "$DB_LOADDIR/$DB_TOPOL" && \
    test_web "$@"
}

# do the test
set +e
if [ $# = 0 ]; then
    # admin_network needs a clean state of database, otherwise it does not work
    test_web_default admin_networks admin_network
    RESULT=$?
    test_web_process || exit
    # default test routine
    if [ "$RESULT" -eq 0 ]; then
    test_web_default -topology -admin_network -admin_networks
    RESULT=$?
    fi
    test_web_process || exit
    if [ "$RESULT" -eq 0 ]; then
	test_web_topo_p topology_power
	RESULT=$?
    fi
    test_web_process || exit
    if [ "$RESULT" -eq 0 ]; then
	test_web_topo_l topology_location
	RESULT=$?
    fi
    test_web_process || exit
else
    # selective test routine
    while [ $# -gt 0 ]; do
	case "$1" in
	    topology_location*)
		test_web_topo_l "$1"
		RESULT=$? ;;
	    topology_power*)
		test_web_topo_p "$1"
		RESULT=$? ;;
	    *)	test_web_default "$1"
		RESULT=$? ;;
	esac
	shift
	test_web_process || exit
	[ "$RESULT" != 0 ] && break
    done
fi
loaddb_default

# cleanup
kill $MAKEPID >/dev/null 2>&1
sleep 2
killall tntnet >/dev/null 2>&1
sleep 2

if [ "$RESULT" = 0 ]; then
    echo "$0: Overall result: SUCCESS"
else
    echo "$0: Overall result: FAILED ($RESULT) seek details above" >&2
fi

exit $RESULT
