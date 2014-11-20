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

[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="@PASSWORD@"

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

BASE_URL="http://127.0.0.1:8000/api/v1"

_TOKEN_=""

print_result() {
    _ret=0
    if [ "$1" -eq 0 ]; then
        echo " * PASSED"
        PASS="`expr $PASS + 1`"
    else
        echo " * FAILED"
        _ret=1
        FAILED="$FAILED $NAME"
    fi
    TOTAL="`expr $TOTAL + 1`"
    echo
    return $_ret
}

test_it() {
    if [ "$1" ]; then
        NAME="$1"
    fi
    [ "$NAME" ] || NAME="`basename "$0" .sh`"
    echo "Running test $NAME:"
}

api_get() {
    curl -v --progress-bar "$BASE_URL$1" 2>&1
}

api_get_json() {
    curl -v --progress-bar "$BASE_URL$1" 2> /dev/null \
    | tr \\n \  | sed -e 's|[[:blank:]]\+||g' -e 's|$|\n|'
}

api_post() {
    curl -v -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

_api_get_token() {
    if [ -z "$_TOKEN_" ]; then
	AUTH_URL="/oauth2/token?username=${BIOS_USER}&password=${BIOS_PASSWD}&grant_type=password"
	_TOKEN_RAW_="`api_get "$AUTH_URL"`"
	_TOKEN_="`echo "$_TOKEN_RAW_" | sed -n 's|.*\"access_token\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'`"
    fi
    echo "$_TOKEN_"
}

api_auth_post() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -d "$2" --progress-bar "$BASE_URL$1" 2>&1
}

api_auth_delete() {
    TOKEN="`_api_get_token`"
    curl -v -H "Authorization: Bearer $TOKEN" -X "DELETE" --progress-bar "$BASE_URL$1" 2>&1
}

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

if ! testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s bios > /dev/null; then
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
cd web/commands
[ "$1" ] || set *
while [ "$1" ]; do
    for NAME in *$1*; do
    . ./"$NAME" 5> "$LOG_DIR/$NAME".log
    if [ -r "../results/$NAME".res ]; then
        diff -Naru "../results/$NAME".res "$LOG_DIR/$NAME".log
        print_result $?
    fi
    done
    shift
done

echo "Testing completed, $PASS/$TOTAL tests passed"
[ -z "$FAILED" ] && exit 0

echo "Following tests failed:"
for i in $FAILED; do
    echo " * $i"
done
exit 1
