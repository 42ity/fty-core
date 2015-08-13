#!/bin/bash

###############################################################################
#                                                                             #
# Copyright (C) 2014 Eaton                                                    #
#                                                                             #
# This program is free software: you can redistribute it and/or modify        #
# it under the terms of the GNU General Public License as published by        #
# the Free Software Foundation; either version 3 of the License, or           #
# (at your option) any later version.                                         #
#                                                                             #
# This program is distributed in the hope that it will be useful,             #
# but WITHOUT ANY WARRANTY; without even the implied warranty of              #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               #
# GNU General Public License for more details.                                #
#                                                                             #
# You should have received a copy of the GNU General Public License           #
# along with this program.  If not, see <http://www.gnu.org/licenses/>.       #
#                                                                             #
###############################################################################

###############################################################################
#                                                                             #
# Author: Karol Hrdina <KarolHrdina@eaton.com>                                #
# Description: rfc-11 admin/iface admin/ifaces automated test                 #
# Version: 1.3                                                                #
# Requirements:                                                               #
#   Following environment variables are expected to be exported:              #
#   SUT_HOST                                                                  #
#   SUT_SSH_PORT                                                              #
#   SUT_WEB_PORT                                                              #
# Todos:                                                                      #
#   flock                                                                     #
###############################################################################

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "FATAL: $0: Could not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "FATAL: $0: Could not include web script library" >&2; exit 1; }

declare -r REST_IFACES="/admin/ifaces"
declare -r REST_IFACE="/admin/iface"
declare -r RESOLV_PATH="/etc"
declare -r RESOLV_FILE="resolv.conf"
declare -r JSON_EXPECTED_FILE="json_expected"
declare -r JSON_RECEIVED_FILE="json_received"
declare -r LOCKFILE="/tmp/ci-test-iface.lock"

usage () {
cat << EOF
USAGE: `basename $0`  

DESCRIPTION:
    Performs automated tests for 'admin/iface', 'admin/ifaces' rest api calls

REQUIREMENTS:
    Following environemnt variables are expected to be set
        SUT_HOST
        SUT_SSH_PORT
        SUT_WEB_PORT
EOF
}

cleanup() {
    local __exit=$?
    rm -f "$LOCKFILE";
    if [[ -n "$TMP_DIR" && -e "$TMP_DIR" ]]; then
        if [ $__exit -ne 0 ]; then
            echo "Test failed => no cleanup; so it's possible to look at files."        
        else
            echo "Cleaning up '$TMP_DIR'"
            rm -rf "$TMP_DIR"
        fi
    fi
    exit $__exit
}

### Script starts here ###
declare -r HOST="$SUT_HOST"
declare -r PORT_SSH="$SUT_SSH_PORT"
declare -r PORT_HTTP="$SUT_WEB_PORT"

if [[ -z "$PORT_SSH" || -z "$PORT_HTTP" || -z "$HOST" ]]; then
    usage
    exit 1
fi

# Previous executions should execute successfully
[ -f "$LOCKFILE" ] && exit 0
trap "cleanup" EXIT
# TODO flock here
# http://stackoverflow.com/questions/169964/how-to-prevent-a-script-from-running-simultaneously
touch "$LOCKFILE"

echo "HOST: '$HOST'"$'\n'"PORT_HTTP: '$PORT_HTTP'"$'\n'"PORT_SSH: '$PORT_SSH'"

# 1. Create temporary dir under /tmp
declare -r TMP_DIR=$(mktemp -d)
if [[ $? -ne 0 ]]; then
    echo "ERROR: Creating temporary directory failed."
    exit 1
fi
echo "Temporary directory created successfully: '${TMP_DIR}'."

# 2. Nameservers
# If /etc/resolv.conf not present it is functionally equivalent to being empty
read -r -d '' SSH_CMD <<EOF-CMD
if [[ ! -e "${RESOLV_PATH}/${RESOLV_FILE}" ]]; then
    touch "${RESOLV_PATH}/${RESOLV_FILE}"
fi
EOF-CMD
ssh -p "${PORT_SSH}" "root@${HOST}" "${SSH_CMD}" || \
    die "ERROR: TODO ssh "
scp -q -P "${PORT_SSH}" "root@${HOST}:${RESOLV_PATH}/${RESOLV_FILE}" "${TMP_DIR}/" || \
    die "ERROR: TODO scp"
# remove comments from the file
#perl -pi -e 's/^#.*\n$//g' "${TMP_DIR}/${RESOLV_FILE_INITIAL}"

nameservers=
while IFS='' read -r line || [[ -n "$line" ]]; do
    if [[ "$line" =~ ^nameserver[[:space:]]+(.+)$ ]]; then
        if [[ -z "$nameservers" ]]; then
            nameservers="\"${BASH_REMATCH[1]}\""
        else
            nameservers="${nameservers}, \"${BASH_REMATCH[1]}\""
        fi
    fi
done < "${TMP_DIR}/${RESOLV_FILE}"

# 3. Interfaces
read -r -d '' SSH_CMD <<EOF-CMD
ip addr > /tmp/ip_addr
EOF-CMD
ssh -p "${PORT_SSH}" "root@${HOST}" "${SSH_CMD}" || \
    die "ERROR: TODO ssh "
scp -q -P "${PORT_SSH}" "root@${HOST}:/tmp/ip_addr" "${TMP_DIR}/ip_addr" || \
    die "ERROR: TODO scp"

declare -a tmp_arr
ip_addr=$(ip addr)
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
bash ./cmpjson.sh "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
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

