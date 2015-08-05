#!/bin/bash

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
    echo "ERROR: vte_test_web.sh is no longer suitable to run all REST API tests"
    echo "       either use ci-test-restapi.sh or specify test on a commandline"
    exit 1
fi
    # *** find the SCRIPTDIR (... test/CI dir) and CHECKOUTDIR

    # *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp|-o|--port)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        --port-web|--sut-port-web|-wp)
            SUT_WEB_PORT="$2"
            shift 2
            ;;
        --host|--machine|-s|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        --sut-user|-su)
            SUT_USER="$2"
            shift 2
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift 2
            ;;
        -p|--passwd|--bios-passwd)
            BIOS_PASSWD="$2"
            shift 2
            ;;
        *)  # fall through
            break
            ;;
    esac
done

# default values:
[ -z "${SUT_USER-}" ] && SUT_USER="root"
[ -z "${SUT_HOST-}" ] && SUT_HOST="debian.roz.lab.etn.com"
# port used for ssh requests:
[ -z "${SUT_SSH_PORT-}" ] && SUT_SSH_PORT="2206"
# port used for REST API requests:
if [ -z "${SUT_WEB_PORT-}" ]; then
    if [ -n "${BIOS_PORT-}" ]; then
        SUT_WEB_PORT="$BIOS_PORT"
    else
        SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
        [ "${SUT_SSH_PORT-}" -ge 2200 ] && \
            SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
    fi
fi

# unconditionally calculated values
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

    # *** if used set BIOS_USER and BIOS_PASSWD for tests where it is used:
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="nosoup4u"

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
# *** include weblib.sh
. "`dirname $0`/weblib.sh" || CODE=$? die "Can not include web script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"


    # *** set PASS and TOTAL to 0
PASS=0
TOTAL=0

echo '*************************************************************************************************************'
logmsg_info "Will use BASE_URL = '$BASE_URL'"
echo $BIOS_USER
echo $BIOS_PASSWD
echo $SUT_HOST
echo $SUT_SSH_PORT
echo $SUT_WEB_PORT


PATH="$PATH:/sbin:/usr/sbin"

    # *** is sasl running on SUT?
if [ "$(sut_run 'pidof saslauthd'|wc -l| sed 's, ,,g')" -gt 0 ];then
    logmsg_info "saslauthd is running"
else
    CODE=1 die "saslauthd is not running, please start it first!"
fi

# is bios user present?
# Check the user account in system
# We expect SASL uses Linux PAM, therefore getent will tell us all we need
LINE="$(sut_run "getent passwd '$BIOS_USER'")"
if [ $? != 0 -o -z "$LINE" ]; then
#if ! getent passwd "$BIOS_USER" > /dev/null; then
    logmsg_error "User $BIOS_USER is not known to system administrative" \
        "database at $SUT_HOST:$SUT_SSH_PORT." \
    logmsg_info "To add it locally, run: "
    echo "    sudo /usr/sbin/useradd --comment 'BIOS REST API testing user' --groups nobody,sasl --no-create-home --no-user-group $BIOS_USER"
    echo "and don't forget the password '$BIOS_PASSWD'"
    CODE=2 die "BIOS_USER absent on remote system"
fi >&2

# is bios access to sasl right?
SASLTEST=$(sut_run "which testsaslauthd")
LINE="$(sut_run "$SASLTEST -u '$BIOS_USER' -p '$BIOS_PASSWD' -s bios")"
if [ $? != 0 -o -z "$LINE" ]; then
    CODE=3 die "SASL autentication for user '$BIOS_USER' has failed." \
        "Please check the existence of /etc/pam.d/bios (and maybe" \
        "/etc/sasl2/bios.conf for some OS distributions)"
fi

# is web server running?
curlfail_push_expect_404
if [ -z "`api_get "" | grep 'HTTP/.* 404 Not Found'`" ]; then
    CODE=4 die "Webserver is not running or has errors, please start it first!"
fi
curlfail_pop

# log dir contents the real responses
cd "`dirname "$0"`"
[ -n "${LOG_DIR-}" ] || LOG_DIR="`pwd`/web/log"
mkdir -p "$LOG_DIR" || exit 4

# cmpjson.sh compares json like files
CMPJSON_SH="`pwd`/cmpjson.sh"
CMPJSON_PY="`pwd`/cmpjson.py"
#[ -z "$CMP" ] && CMP="`pwd`/cmpjson.py"
[ -z "${CMP-}" ] && CMP="$CMPJSON_SH"
[ -s "${CMP-}" ] || exit 5

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

# TODO: Port recent changes from main test_web.sh
# ... or merge these two via sut_run() commands etc.

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
