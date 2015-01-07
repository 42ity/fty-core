#!/bin/sh

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
# Author(s): Michal Hrusecky <MichalHrusecky@eaton.com>,
#            Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: This script automates tests of REST API for the $BIOS project

PASS=0
TOTAL=0

[ "x$CHECKOUTDIR" = "x" ] && \
    case "`dirname $0`" in
	*/tests/CI|tests/CI)
	   CHECKOUTDIR="$( cd `dirname $0`; pwd | sed 's|/tests/CI$||' )" || \
	   CHECKOUTDIR="" ;;
    esac
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

. "`dirname $0`/weblib.sh" || exit $?

while [ $# -gt 0 ]; do
    case "$1" in
    -u|--user)
        BIOS_USER="$2"
        shift 2
        ;;
    -p|--passwd)
        BIOS_PASSWD="$2"
        shift 2
        ;;
    *)
        break
        ;;
    esac
done

PATH="$PATH:/sbin:/usr/sbin"

# fixture ini
if ! pidof saslauthd > /dev/null; then
    echo "saslauthd is not running, please start it first!" >&2
    exit 1
fi

# Check the user account in system
# We expect SASL uses Linux PAM, therefore getent will tell us all we need
if ! getent passwd "$BIOS_USER" > /dev/null; then
    echo "User $BIOS_USER is not known to system administrative database"
    echo "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
    echo "and don't forget the password '$BIOS_PASSWD'"
    exit 2
fi

SASLTEST="`which testsaslauthd`"
[ -x "$SASLTEST" ] || SASLTEST="/usr/sbin/testsaslauthd"
[ -x "$SASLTEST" ] || SASLTEST="/sbin/testsaslauthd"

if ! $SASLTEST -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios > /dev/null; then
    echo "SASL autentication for user '$BIOS_USER' has failed. Check the existence of /etc/pam.d/bios (and maybe /etc/sasl2/bios.conf for some OS distributions)"
    exit 3
fi

if [ -z "`api_get "" | grep "< HTTP/.* 404 Not Found"`" ]; then
    echo "Webserver is not running, please start it first!"
    exit 4
fi

cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "$CMP" ] && CMP="`pwd`/cmpjson.sh"
[ -s "$CMP" ] || exit 5
cd web/commands
POSITIVE=""
NEGATIVE=""
while [ "$1" ]; do
    if [ -z "`echo "x$1" | grep "^x-"`" ]; then
        POSITIVE="$POSITIVE $1"
    else
        NEGATIVE="$NEGATIVE `echo "x$1" | sed 's|^x-||'`"
    fi
    shift
done
[ -n "$POSITIVE" ] || POSITIVE="*"
for i in $POSITIVE; do
    for NAME in *$i*; do
    SKIP=""
    for n in $NEGATIVE; do
        if expr match $NAME .\*"$n".\* > /dev/null; then
            SKIP="true"
        fi
    done
    [ -z "$SKIP" ] || continue
    . ./"$NAME" 5> "$LOG_DIR/$NAME".log
    if [ -r "../results/$NAME".res ]; then
        RESULT="../results/$NAME".res
        EXPECT="$LOG_DIR/$NAME".log
        if [ -x "../results/$NAME".cmp ]; then
            ../results/"$NAME".cmp "$RESULT" "$EXPECT"
        else
            "$CMP" "$RESULT" "$EXPECT"
        fi
        RES=$?
        if [ $RES -ne 0 ]; then
            diff -Naru "../results/$NAME".res "$LOG_DIR/$NAME".log
        fi
        print_result $RES
    fi
    done
done

echo "Testing completed, $PASS/$TOTAL tests passed"
[ -z "$FAILED" ] && exit 0

echo "Following tests failed:"
for i in $FAILED; do
    echo " * $i"
done
exit 1
