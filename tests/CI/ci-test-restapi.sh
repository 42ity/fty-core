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
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: tests restapi

[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
USER=bios
PASSWD=@PASSWORD@

usage(){
    echo "usage $(basename $0) [options]"
    echo "options:"
    echo "  -u|--user   username for SASL [bios]"
    echo "  -p|--passwd password for SASL [@PASSWORD@]"
}

while [ $# -gt 0 ] ; do
    case "$1" in
        --user|-u)
            USER="$2"
            shift
            ;;
        --passwd|-p)
            PASSWD="$2"
            shift
            ;;
        *)
            echo "Invalid option $1" 1>&2
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
  # might have some mess
  killall tntnet 2>/dev/null || true
  # make sure sasl is running
  systemctl restart saslauthd
  # check SASL is working
  testsaslauthd -u "$USER" -p "$PASSWD" -s bios

# do the webserver
  cd $CHECKOUTDIR
  # make clean
  export USER PASSWD
  make web-test &
  MAKEPID=$!
  wait_for_web

# do the test
set +e
echo "============================================================"
/bin/bash tests/CI/test_web.sh
RESULT=$?
echo "============================================================"

# cleanup
kill $MAKEPID 2>/dev/null
sleep 2
killall tntnet 2>/dev/null
sleep 2
exit $RESULT
