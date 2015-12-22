#!/bin/bash

# Copyright (c) 2014 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#  \file    ci-test-iface.sh
#  \brief   rfc-11 admin/iface admin/ifaces automated test
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \version 1.3

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "FATAL: $0: Could not include script library" >&2; exit 1; }

# Set up weblib test engine
WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="ignore" # we do this in-script
WEBLIB_QUICKFAIL=no
WEBLIB_CURLFAIL=no
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_QUICKFAIL WEBLIB_CURLFAIL

# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "FATAL: $0: Could not include web script library" >&2; exit 1; }

# Setting BUILDSUBDIR and CHECKOUTDIR
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the `basename $0` REST API webserver"

PATH="/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

declare -r REST_IFACES="/admin/ifaces"
declare -r REST_IFACE="/admin/iface"

declare -r RESOLV_PATH="/etc"
declare -r RESOLV_FILE="resolv.conf"

declare -r JSON_EXPECTED_FILE="json_expected"
declare -r JSON_RECEIVED_FILE="json_received"

declare -r LOCKFILE="/tmp/ci-test-iface.lock"

test_web_port() {
    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}
test_web_process() {
    [ -z "$MAKEPID" ] && return 0

    if [ ! -d /proc/$MAKEPID ]; then
        logmsg_error "Web-server process seems to have died!" >&2
        # Ensure it is dead though, since we abort the tests now
        kill $MAKEPID >/dev/null 2>&1
        RES_TWP=32
        wait $MAKEPID >/dev/null 2>&1 || RES_TWP=$?
        return $RES_TWP
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
    logmsg_error "Port ${SUT_WEB_PORT} still not in LISTEN state" >&2
    return 1
}

cleanup() {
    local __exit=$?
    rm -f "$LOCKFILE"
    if [[ -n "$TMP_DIR" && -e "$TMP_DIR" ]]; then
        if [ $__exit -ne 0 ]; then
            echo "Test failed => no cleanup; so it's possible to look at files."        
        else
            echo "Cleaning up '$TMP_DIR'"
            rm -rf "$TMP_DIR"
        fi
    fi
    if [ -n "$MAKEPID" -a -d "/proc/$MAKEPID" ]; then
        logmsg_info "Killing make web-test PID $MAKEPID to exit."
        kill -INT "$MAKEPID"
    fi
    killall -INT tntnet 2>/dev/null
    sleep 1
    killall      tntnet 2>/dev/null
    sleep 1
    ps -ef | grep -v grep | egrep "tntnet" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "tntnet still alive, trying SIGKILL" && \
        { killall -KILL tntnet 2>/dev/null ; exit 1; }

    exit $__exit
}

### Environment preparation ###
# Assumptions:
#   `ci-rc-bios.sh --stop` is called prior to invoking this script
#   script is called as root

# Previous executions should execute successfully
[ -f "$LOCKFILE" ] && exit 0
trap "cleanup" EXIT
touch "$LOCKFILE"

test_web_port && die "Port ${SUT_WEB_PORT} is in LISTEN state when it should be free."

# make sure sasl is running
if ! systemctl --quiet is-active saslauthd; then
    logmsg_info "Starting saslauthd..."
    systemctl start saslauthd || die "Could not start saslauthd."
fi

# check sasl is working
testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s "$SASL_SERVICE" || \
    die "saslauthd is NOT responsive or not configured!"

# make sure database is running
if ! systemctl --quiet is-active mysql; then
    logmsg_info "Starting mysql..."
    systemctl start mysql || die "Could not start mysql."
fi

# do the webserver
LC_ALL=C
LANG=C
export BIOS_USER BIOS_PASSWD SASL_SERVICE LC_ALL LANG
if [ ! -x "${BUILDSUBDIR}/config.status" ]; then
    ./autogen.sh --nodistclean --configure-flags \
    "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
    ${AUTOGEN_ACTION_CONFIG} || exit
fi
./autogen.sh ${AUTOGEN_ACTION_MAKE} V=0 web-test-deps || exit

./autogen.sh --noparmake ${AUTOGEN_ACTION_MAKE} web-test &
MAKEPID=$!

wait_for_web || die "Web-server is NOT responsive!"
sleep 5
test_web_process || die "test_web_process() failed."

### Script starts here ###

# 1. Create temporary dir under /tmp
declare -r TMP_DIR=$(mktemp -d)
if [[ $? -ne 0 ]]; then
    echo "ERROR: Creating temporary directory failed."
    exit 1
fi
echo "Temporary directory created successfully: '${TMP_DIR}'."

# 2. Nameservers
# If /etc/resolv.conf not present it is functionally equivalent to being empty
if [[ ! -e "${RESOLV_PATH}/${RESOLV_FILE}" ]]; then
    touch "${TMP_DIR}/${RESOLV_FILE_INITIAL}"
else
    cp -f "${RESOLV_PATH}/${RESOLV_FILE}" "${TMP_DIR}/${RESOLV_FILE_INITIAL}" || \
        die "copying '${RESOLV_PATH}/${RESOLV_FILE}' to '${TMP_DIR}/${RESOLV_FILE_INITIAL}' failed."
fi
# Remove comments
perl -pi -e 's/^#.*\n$//g' "${TMP_DIR}/${RESOLV_FILE_INITIAL}"

nameservers=
while IFS='' read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^nameserver[[:space:]]+(.+)$ ]]; then
        if [[ -z "$nameservers" ]]; then
            nameservers="\"${BASH_REMATCH[1]}\""
        else
            nameservers="${nameservers}, \"${BASH_REMATCH[1]}\""
        fi
    fi
done < "${TMP_DIR}/${RESOLV_FILE_INITIAL}"

# 3. Interfaces
ip addr > "${TMP_DIR}/ip_addr" || \
    die "Error executing 'ip addr > \"${TMP_DIR}/ip_addr\"'."

declare -a tmp_arr
while read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^[[:space:]]*[0-9]+:[[:space:]]+([[:alnum:]]+): ]]; then
        tmp_arr+=( "${BASH_REMATCH[1]}" )
    fi
done < "${TMP_DIR}/ip_addr"
declare -ar INTERFACES=("${tmp_arr[@]}")
echo "Interfaces=${INTERFACES[@]}"

# TEST CASES

####################
### admin/ifaces ###
####################
TEST_CASE="admin/ifaces"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="
tmp='{ "ifaces": [ '
counter=0
for i in "${INTERFACES[@]}"; do
    if [[ $counter -eq 0 ]]; then
        counter=1
        tmp="${tmp} \"${i}\""
        continue
    fi
    tmp="${tmp}, \"${i}\""
done
tmp="${tmp} ] }"
echo "$tmp" > "${TMP_DIR}/${JSON_EXPECTED_FILE}"
HTTP_CODE=
simple_get_json_code "${REST_IFACES}" tmp HTTP_CODE || die "'api_get_json ${REST_IFACES}' failed."
echo "$tmp" > "${TMP_DIR}/${JSON_RECEIVED_FILE}"
bash "${CHECKOUTDIR}/tests/CI/cmpjson.sh" "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
    die "Test case '$TEST_CASE' failed. Expected and returned json do not match."
[[ $HTTP_CODE -eq 200 ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 200, received: $HTTP_CODE."

echo "SUCCESS"


################################
### admin/iface/<iface_name> ###
################################
# TODO Not finished yet
TEST_CASE="admin/iface/"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="
read -r -d '' TEMPLATE <<'EOF-TMPL'
{
    "##IFACE##" : {
        "state" : "##STATE##",
        "ip" : ##IP##,
        "gateway" : ##GATEWAY##,
        "link" : ##LINK##,
        "mac" : ##MAC##,
        "nameservers" : [ ##NAMESERVERS##  ]
    }
}
EOF-TMPL
echo "IMPLEMETATION WIP"
#for iface in "${INTERFACES[@]}"; do
#done

