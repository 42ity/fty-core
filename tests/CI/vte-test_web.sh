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
#            Radomir Vrajik <RadomirVrajik@eaton.com>
# Description: This script automates tests of REST API for the $BIOS project
# It works with the image instalation of the BIOS on SUT (System Under Test) 
# server and is started on another server - MS (test Management Station)
#
# ***** ABBREVIATIONS *****
    # *** abbreviation SUT - System Under Test - remote server with BIOS ***
    # *** abbreviation MS - Management Station - local server with this script ***

# ***** PREREQUISITES *****
    # *** dealing with external parameter, some are mandatory. Te script must be
        # * The script must be called in the following format:
        # * test_web.sh -u "<user on SUT>" -p "<pasword>" \
        # * "<substring for the testcase filtering>"
   # *** no parameters? ERROR!
if [ $# -eq 0 ]; then
    echo "ERROR: test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi
    # *** set PASS and TOTAL to 0
PASS=0
TOTAL=0

    # *** find the SCRIPTDIR (... test/CI dir) and CHECKOUTDIR
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

    # *** include weblib.sh
. "`dirname $0`/weblib.sh" || exit $?

    # *** Set BIOS_USER,BIOS PASSWD,SUT_NAME and SUT_PORT from parameters
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
    -s|--sut)
        SUT_NAME="$2"
        shift 2
        ;;
    -o|--port)
        SUT_PORT="$2"
        shift 2
        ;;
    *)
        break
        ;;
    esac
done


SUT_HTTP_PORT=$(expr $SUT_PORT - 2200 + 8000)
echo '*************************************************************************************************************'
echo $BIOS_USER
echo $BIOS_PASSWD
echo $SUT_NAME
echo $SUT_PORT
echo $SUT_HTTP_PORT


BASE_URL="http://$SUT_NAME:$SUT_HTTP_PORT/api/v1"
PATH="$PATH:/sbin:/usr/sbin"

    # *** is sasl running on SUT?
if [ echo $(ssh -p $SUT_PORT $SUT_NAME "pidof saslauthd"|wc -l &)>/dev/null ];then
#if [ $(echo $(ssh -p 2208 root@debian.roz.lab.etn.com "pidof saslauthd" &))>/dev/null ];then
    echo "saslauthd is running"
else
    echo "saslauthd is not running, please start it first!" >&2
    exit 1
fi

# is bios user present?
# Check the user account in system
# We expect SASL uses Linux PAM, therefore getent will tell us all we need
if [ ! echo $(ssh -p $SUT_PORT $SUT_NAME "getent passwd '$BIOS_USER'" &)>/dev/null ];then
#if ! getent passwd "$BIOS_USER" > /dev/null; then
    echo "User $BIOS_USER is not known to system administrative database"
    echo "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
    echo "and don't forget the password '$BIOS_PASSWD'"
    exit 2
fi

# is bios access to sasl right?
SASLTEST=$(ssh -p $SUT_PORT $SUT_NAME "which testsaslauthd" &)

if ! echo $(ssh -p $SUT_PORT $SUT_NAME "$SASLTEST -u '$BIOS_USER' -p '$BIOS_PASSWD'" &) -s bios > /dev/null; then
    echo "SASL autentication for user '$BIOS_USER' has failed. Check the existence of /etc/pam.d/bios (and maybe /etc/sasl2/bios.conf for some OS distributions)"
    exit 3
fi

# is web server running?
if [ -z "`api_get "" | grep "< HTTP/.* 404 Not Found"`" ]; then
    echo "Webserver is not running, please start it first!"
    exit 4
fi

# log dir contents the real responses
cd "`dirname "$0"`"
[ "$LOG_DIR" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4

# cmpjson.sh compares json like files
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "$CMP" ] && CMP="$CMPJSON_SH"
[ -s "$CMP" ] || exit 5

# web/commands dir contains the request commands
cd web/commands

# positive parameters are included to test, negative excluded
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

# if POSITIVE parameters variable is empty, then all tests are included
[ -n "$POSITIVE" ] || POSITIVE="*"

# A bash-ism, should set the exitcode of the rightmost failed command
# in a pipeline, otherwise e.g. exitcode("false | true") == 0
set -o pipefail 2>/dev/null || true

for i in $POSITIVE; do
    for NAME in *$i*; do
    SKIP=""
    for n in $NEGATIVE; do
        if expr match $NAME .\*"$n".\* > /dev/null; then
            SKIP="true"
        fi
    done
    [ -z "$SKIP" ] || continue
# start testcase $NAME and put the result to $NAME.log
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
