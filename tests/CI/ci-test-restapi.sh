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
# the $BIOS project (Note: please also run `ci-fill-db.sh` beforehand!)

[ "x$CHECKOUTDIR" = "x" ] && \
    case "`dirname $0`" in
       */tests/CI|tests/CI)
           CHECKOUTDIR="$( cd `dirname $0`; pwd | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"

PATH=/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

# Simple check for whether sudo is needed to restart saslauthd
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

usage(){
    echo "Usage: $(basename $0) [options...]"
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
        *)
            echo "Invalid option '$1'" >&2
            usage
            exit 1
            ;;
    esac
    shift
done

set -u
set -e

# TODO: port 8000 as a param?
wait_for_web() {
    for a in $(seq 60) ; do
        sleep 5
        if ( netstat -tan | grep -w 8000 >/dev/null ) ; then
            return 0
        fi
    done
    echo "ERROR: Port 8000 still not in LISTEN state" >&2
    return 1
}


# prepare environment
  cd $CHECKOUTDIR || { echo "FATAL: Unusable CHECKOUTDIR='$CHECKOUTDIR'" >&2; exit 1; }

  # might have some mess
  killall tntnet 2>/dev/null || true
  # make sure sasl is running
  $RUNAS systemctl restart saslauthd || \
    [ x"$RUNAS" = x ] || \
    echo "WARNING: Could not restart saslauthd, make sure SASL and SUDO are installed and /etc/sudoers.d/bios_01_citest is set up per INSTALL docs" >&2
  # check SASL is working
  testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios

# do the webserver
  # make clean
  export BIOS_USER BIOS_PASSWD
  make web-test &
  MAKEPID=$!
  wait_for_web

# do the test
set +e
echo "============================================================"
/bin/bash tests/CI/test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD"
RESULT=$?
echo "============================================================"

# cleanup
kill $MAKEPID 2>/dev/null
sleep 2
killall tntnet 2>/dev/null
sleep 2
exit $RESULT
