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

if [ $# -eq 0 ]; then
    echo "ERROR: test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi

PASS=0
TOTAL=0

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
NEED_BUILDSUBDIR=no determineDirs_default || true
#cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"


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
    CODE=1 die "saslauthd is not running, please start it first!"
fi

# Check the user account in system
# We expect SASL uses Linux PAM, therefore getent will tell us all we need
if ! getent passwd "$BIOS_USER" > /dev/null; then
    CODE=2 die "User $BIOS_USER is not known to system administrative database" \
        "To add it locally, run: "
        "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
        "and don't forget the password '$BIOS_PASSWD'"
    exit 2
fi

SASLTEST="`which testsaslauthd`"
[ -x "$SASLTEST" ] || SASLTEST="/usr/sbin/testsaslauthd"
[ -x "$SASLTEST" ] || SASLTEST="/sbin/testsaslauthd"

if ! $SASLTEST -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios > /dev/null; then
    CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
        "Check the existence of /etc/pam.d/bios (and maybe /etc/sasl2/bios.conf for some OS distributions)"
fi

if [ -z "`api_get "" | grep "< HTTP/.* 404 Not Found"`" ]; then
    api_get "" >&2
    CODE=4 die "Webserver is not running or code is broken, please start it first!"
fi

cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "$CMP" ] && CMP="$CMPJSON_SH"
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

# A bash-ism, should set the exitcode of the rightmost failed command
# in a pipeline, otherwise e.g. exitcode("false | true") == 0
set -o pipefail 2>/dev/null || true

for i in $POSITIVE; do
    for NAME in *$i*.sh; do
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

logmsg_info "Testing completed, $PASS/$TOTAL tests passed"
[ -z "$FAILED" ] && exit 0

logmsg_error "The following tests have failed:"
for i in $FAILED; do
    echo " * $i"
done
exit 1
