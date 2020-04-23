#!/bin/bash

# Copyright (c) 2014 - 2020 Eaton
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
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT

# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "FATAL: $0: Could not include web script library" >&2; exit 1; }

# Setting BUILDSUBDIR and CHECKOUTDIR
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the `basename $0` REST API webserver"

PATH="$BUILDSUBDIR/tools:$CHECKOUTDIR/tools:${DESTDIR:-/root}/libexec/bios:/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

declare -r REST_IFACES="/admin/ifaces"
declare -r REST_IFACE="/admin/iface"

declare -r RESOLV_PATH="/etc"
declare -r RESOLV_FILE="resolv.conf"
declare -r RESOLV_FILE_INITIAL="$RESOLV_FILE.initial"

declare -r JSON_EXPECTED_FILE="json_expected"
declare -r JSON_RECEIVED_FILE="json_received"

declare -r LOCKFILE="/tmp/ci-test-iface.lock"

test_web_port() {
    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}
test_web_process() {
    [ -z "$MAKEPID" ] && return 0

    if [ ! "-d /proc/$MAKEPID" ]; then
        logmsg_error "Web-server process seems to have died!" >&2
        # Ensure it is dead though, since we abort the tests now
        kill "$MAKEPID" >/dev/null 2>&1
        RES_TWP=32
        wait "$MAKEPID" >/dev/null 2>&1 || RES_TWP=$?
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

kill_daemons() {
    if [ -n "$MAKEPID" -a -d "/proc/$MAKEPID" ]; then
        logmsg_info "Killing make web-test PID $MAKEPID to exit"
        kill -INT "$MAKEPID"
    fi
    killall -INT tntnet 2>/dev/null
    sleep 1
    killall      tntnet 2>/dev/null
    sleep 1
    ps -ef | grep -v grep | egrep "tntnet" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "tntnet still alive, trying SIGKILL" && \
        { killall -KILL tntnet 2>/dev/null ; return 1; }
    return 0
}

cleanup() {
    local __exit="$1"
    rm -f "$LOCKFILE"
    if [[ -n "${TMP_DIR-}" ]] && [[ -e "$TMP_DIR" ]]; then
        if [ $__exit -ne 0 ]; then
            logmsg_error "Test failed => no cleanup; so it's possible to look at files."
        else
            logmsg_info "Cleaning up '$TMP_DIR'"
            rm -rf "$TMP_DIR"
        fi
    fi
    kill_daemons || return $?
    return $__exit
}

### Environment preparation ###
# Assumptions:
#   `ci-rc-bios.sh --stop` is called prior to invoking this script
#   script is called as root

# Previous executions should execute successfully
# ERRCODE is maintained by scriptlib::settraps()
settraps 'cleanup $ERRCODE || ERRCODE=$?; exit_summarizeTestlibResults $ERRCODE'

[ -f "$LOCKFILE" ] && die "Previous executions should execute successfully first"
touch "$LOCKFILE"

test_web_port && die "Port ${SUT_WEB_PORT} is in LISTEN state when it should be free."

# make sure sasl is running
if ! systemctl is-active saslauthd --quiet; then
    logmsg_info "Starting saslauthd..."
    systemctl start saslauthd || die "Could not start saslauthd"
fi

# check sasl is working
testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s "$SASL_SERVICE" || \
    die "saslauthd is NOT responsive or not configured!"

# make sure database is running
if ! systemctl is-active mysql --quiet; then
    logmsg_info "Starting mysql..."
    systemctl start mysql || die "Could not start mysql"
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
test_web_process || die "test_web_process() failed"

### Script starts here ###

# 1. Create temporary dir under /tmp
declare -r TMP_DIR="$(mktemp -d)" && [ -n "$TMP_DIR" ] && [ -d "$TMP_DIR" ] || \
    CODE=$? die "Creating temporary directory failed"
logmsg_info "Temporary directory created successfully: '${TMP_DIR}'"

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
test_it "get_ip_addr"
ip addr > "${TMP_DIR}/ip_addr" || \
    die "Error executing 'ip addr > \"${TMP_DIR}/ip_addr\"'."
print_result 0

declare -a tmp_arr
while read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^[[:space:]]*[0-9]+:[[:space:]]+([[:alnum:]]+): ]]; then
        tmp_arr+=( "${BASH_REMATCH[1]}" )
    fi
done < "${TMP_DIR}/ip_addr"
declare -ar INTERFACES=("${tmp_arr[@]}")
logmsg_info "Interfaces=${INTERFACES[@]}"

# TEST CASES

logmsg_info '####################'
logmsg_info '### admin/ifaces ###'
logmsg_info '####################'

test_it "admin/ifaces:got_some_ifaces_local"
counter=0
tmp=''
for i in "${INTERFACES[@]}"; do
    if [[ $counter -eq 0 ]]; then
        counter=1
        tmp="\"${i}\""
        continue
    fi
    tmp="${tmp}, \"${i}\""
done
[ -n "$tmp" ] && [ -n "`echo "$tmp" | sed 's,[\ \,\"],,g'`" ]
print_result $? "Nothing found in the INTERFACES array"
tmp='{ "ifaces": [ '"${tmp}"' ] }'

test_it "admin/ifaces:got_some_ifaces_restapi"
RES=0
curlfail_push_expect_noerrors
api_get_json "${REST_IFACES}" > "${TMP_DIR}/${JSON_RECEIVED_FILE}" || RES=$?
curlfail_pop
print_result $RES

test_it "admin/ifaces:compare_lists_of_ifaces"
RES=0
echo "$tmp" > "${TMP_DIR}/${JSON_EXPECTED_FILE}" || RES=$?
"${CHECKOUTDIR}/tests/CI/cmpjson.sh" -f "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || RES=$?
print_result $RES || ls -la "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}"

# The trap-handler should display the summary (if any)
exit 0

######################################################

logmsg_info '################################'
logmsg_info '### admin/iface/<iface_name> ###'
logmsg_info '################################'
logmsg_info '#### TODO: Not finished yet ####'
logmsg_info '################################'

test_it "admin/iface/"
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
logmsg_info "IMPLEMENTATION WIP"
#for iface in "${INTERFACES[@]}"; do
#done
print_result 0

# The trap-handler should display the summary (if any)
exit 0
