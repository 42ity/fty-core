#!/bin/bash
#
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
#
#! \file    ci-test-netcfg.sh
#  \brief   rfc-11 admin/netcfg admin/netcfgs automated test
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \version 1.3
#
# Requirements:
#   Following environment variables are expected to be exported:
#   SUT_HOST
#   SUT_SSH_PORT
#   SUT_WEB_PORT
# Todos:
#   flock
#   netcfg PUT tests - license issue

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "FATAL: $0: Could not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "FATAL: $0: Could not include web script library" >&2; exit 1; }

declare -r REST_NETCFGS="/admin/netcfgs"
declare -r REST_NETCFG="/admin/netcfg"
declare -r IFACES_PATH="/etc/network"
declare -r IFACES_FILE="interfaces"
declare -r IFACES_FILE_INITIAL="${IFACES_FILE}_initial"
declare -r RESOLV_PATH="/etc"
declare -r RESOLV_FILE="resolv.conf"
declare -r RESOLV_FILE_INITIAL="${RESOLV_FILE}_initial"
declare -r JSON_EXPECTED_FILE="json_expected"
declare -r JSON_RECEIVED_FILE="json_received"
declare -r LOCKFILE="/tmp/ci-test-netcfg.lock"

usage () {
cat << EOF
USAGE: `basename $0`  

DESCRIPTION:
    Performs automated tests for 'admin/netcfg', 'admin/netcfgs' rest api calls

REQUIREMENTS:
    Following environemnt variables are expected to be set
        SUT_HOST
        SUT_SSH_PORT
        SUT_WEB_PORT
EOF
}

# Restore config file on remote host to initial state
#
# Expectes the following variables to be non-empty: TMP_DIR, HOST, PORT_SSH
# Arguments
#   $1 - config file name; accepts: "interfaces", "resolv.conf"
# Returns:
#   1 - error
#   0 - success
restore_config() {
    # arguments check
    [[ -z "$TMP_DIR" || -z "$HOST" || -z "$PORT_SSH" ]] && return 1
    if [ $# -lt 1 ]; then
        logmsg_error "restore_config(): wrong number of arguments."
        return 1
    fi
    if [[ "$1" != "$IFACES_FILE" && "$1" != "$RESOLV_FILE" ]]; then
        logmsg_error "restore_config(): bad argument."
        return 1
    fi
    [[ ! -e "${TMP_DIR}" ]]

    local __remote=
    local __initial="${TMP_DIR}"
    case "$1" in
        "$IFACES_FILE")
            __remote="${IFACES_PATH}/${IFACES_FILE}"
            __initial="${__initial}/${IFACES_FILE_INITIAL}"
            ;;
        "$RESOLV_FILE")
            __remote="${RESOLV_PATH}/${RESOLV_FILE}"
            __initial="${__initial}/${RESOLV_FILE_INITIAL}"
            ;;
    esac
    scp -q -P "${PORT_SSH}" "${__initial}" "root@${HOST}:${__remote}"
    if [[ $? -ne 0 ]]; then
        logmsg_error "restore_config(): restoring config file on ${__remote} on ${HOST} failed; scp failed."
        return 1
    else
        return 0
    fi
}

array_contains () {
    # TODO arg check?
    local __arr_name="$1[@]"
    local __needle="$2"
    local __element=
    local __contains=1;
    for __element in "${!__arr_name}"; do
        if [[ "${__element}" == "${__needle}" ]]; then
            __contains=0
            break
        fi
    done
    return $__contains
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

# Placeholder

# 0. Sanity check
read -r -d '' SSH_CMD <<EOF-CMD
if [[ ! -e "${IFACES_PATH}/${IFACES_FILE}" ]]; then
    exit 1
fi
EOF-CMD
ssh -p "${PORT_SSH}" "root@${HOST}" "${SSH_CMD}" || \
    die "ssh connection error OR  '${IFACES_PATH}/${IFACES_FILE}' not found on remote '$HOST'."

# 1. Create temporary dir under /tmp
declare -r TMP_DIR=$(mktemp -d)
if [[ $? -ne 0 ]]; then
    echo "ERROR: Creating temporary directory failed."
    exit 1
fi
echo "Temporary directory created successfully: '${TMP_DIR}'."

# 2. Store the initial /etc/resolv.conf
# If /etc/resolv.conf not present it is functionally equivalent to being empty
read -r -d '' SSH_CMD <<EOF-CMD
if [[ ! -e "${RESOLV_PATH}/${RESOLV_FILE}" ]]; then
    touch "${RESOLV_PATH}/${RESOLV_FILE}"
fi
EOF-CMD
ssh -p "${PORT_SSH}" "root@${HOST}" "${SSH_CMD}" || \
    die "ERROR: TODO ssh "
scp -q -P "${PORT_SSH}" "root@${HOST}:${RESOLV_PATH}/${RESOLV_FILE}" "${TMP_DIR}/${RESOLV_FILE_INITIAL}" || \
    die "ERROR: TODO scp"
perl -pi -e 's/^#.*\n$//g' "${TMP_DIR}/${RESOLV_FILE_INITIAL}"

# 3. Store the initial /etc/network/interfaces
scp -q -P "${PORT_SSH}" "root@${HOST}:${IFACES_PATH}/${IFACES_FILE}" "${TMP_DIR}/${IFACES_FILE_INITIAL}" || \
    die "ERROR: scp failed"
perl -pi -e 's/^#.*\n$//g' "${TMP_DIR}/${IFACES_FILE_INITIAL}"

# 3.1 Parse out into array iface names of the initial /etc/network/interfaces
declare -a tmp_arr
while IFS='' read -r line || [[ -n "$line" ]]; do
    if [[ $line =~ ^[[:space:]]*iface[[:space:]]+([[:alnum:]]+)[[:space:]]+ ]]; then
        tmp_arr+=( "${BASH_REMATCH[1]}" )
#        for i in $line; do
#            tmp_arr+=($i)
#        done
    fi
done < "${TMP_DIR}/${IFACES_FILE_INITIAL}"
declare -ar INITIAL_IFACE_NAMES=("${tmp_arr[@]}")
echo "Initial iface names: ${INITIAL_IFACE_NAMES[@]}"
if [ ${#INITIAL_IFACE_NAMES[@]} -lt 1 ]; then
    die "No interfaces available"
fi

# TEST CASES

########################
### Netcfgs_Read_001 ###
########################
TEST_CASE="Netcfgs_Read_001"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="
tmp='{ "netcfgs": [ '
counter=0
for i in "${INITIAL_IFACE_NAMES[@]}"; do
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
simple_get_json_code "${REST_NETCFGS}" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
echo "$tmp" > "${TMP_DIR}/${JSON_RECEIVED_FILE}"
bash ./cmpjson.sh "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
    die "Test case '$TEST_CASE' failed. Expected and returned json do not match."
[[ $HTTP_CODE -eq 200 ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 200, received: $HTTP_CODE."
# Repeated requests
for i in "1 2 3 4 5"; do 
    simple_get_json_code "${REST_NETCFGS}" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
    echo "$tmp" > "${TMP_DIR}/${JSON_RECEIVED_FILE}"
    bash ./cmpjson.sh -f "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
        die "Test case '$TEST_CASE' failed. Expected and returned json do not match."
    [[ $HTTP_CODE -eq 200 ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 200, received: $HTTP_CODE."
done
echo "SUCCESS"

########################
### Netcfgs_Read_002 ###
########################
TEST_CASE="Netcfgs_Read_002"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="
# Make a copy of the interfaces_initial that we'll modify
cat "${TMP_DIR}/${IFACES_FILE_INITIAL}" > "${TMP_DIR}/${IFACES_FILE}"

for i in "${INITIAL_IFACE_NAMES[@]}"; do
    # remove interface ${i} from 'tmp/tmp.XXXXX/interfaces'
    perl -pi -e "s/auto(.+?)${i}(.*?)\n/auto\1\2\n/gs" "${TMP_DIR}/${IFACES_FILE}"
    perl -pi -e "s/allow-hotplug(.+?)${i}(.*?)\n/allow-hotplug\1\2\n/gs" "${TMP_DIR}/${IFACES_FILE}"
    perl -pi -e "BEGIN{undef $/;} s/iface(\s+)${i}.*?(iface|\Z)/\2/gs" "${TMP_DIR}/${IFACES_FILE}"
    perl -pi -e "s/allow-hotplug\s*\n*$//gs" "${TMP_DIR}/${IFACES_FILE}"
    perl -pi -e "s/^auto\s*\n*$//gs" "${TMP_DIR}/${IFACES_FILE}"
    perl -pi -e "s/^[\s\n]*\Z//gs" "${TMP_DIR}/${IFACES_FILE}"

    scp -q -P "${PORT_SSH}" "${TMP_DIR}/${IFACES_FILE}" "root@${HOST}:${IFACES_PATH}/${IFACES_FILE}" || \
        die "scp TODO"
    simple_get_json_code "${REST_NETCFGS}" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
    echo "$tmp" > "${TMP_DIR}/${JSON_RECEIVED_FILE}"
    perl -pi -e "s/,\s*\"${i}\"//g" "${TMP_DIR}/${JSON_EXPECTED_FILE}"
    perl -pi -e "s/\"${i}\"\s*,//g" "${TMP_DIR}/${JSON_EXPECTED_FILE}"
    perl -pi -e "s/\[\s*\"${i}\"\s*\]/[]/g" "${TMP_DIR}/${JSON_EXPECTED_FILE}"
    if [[ -s "${TMP_DIR}/${IFACES_FILE}" ]]; then    
        bash ./cmpjson.sh -f "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
            die "Test case '$TEST_CASE' failed. Expected and returned json do not match."
        [ $HTTP_CODE -eq 200  ] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 200, received: $HTTP_CODE."
    else
        # if /tmp/tmp.XXXX/interfaces empty then 404 expected
        [ $HTTP_CODE -eq 404  ] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 404, received: $HTTP_CODE."
    fi
done
restore_config "$IFACES_FILE" || die "Restoring $IFACES_FILE failed."
echo "SUCCESS"

########################
### Netcfgs_Read_003 ###
########################
TEST_CASE="Netcfgs_Read_003"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="

simple_get_json_code "${REST_NETCFGS}/advdwsqwe234?=345" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
HTTP_EXPECTED=404
[ $HTTP_CODE -eq 404 -o $HTTP_CODE -eq 401 ] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 404 or 401, received: $HTTP_CODE."

simple_get_json_code "${REST_NETCFGS}?a=b&c=d" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
HTTP_EXPECTED=200
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."
echo "SUCCESS"

#######################
### Netcfg_Read_001 ###
#######################
TEST_CASE="Netcfg_Read_001"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="
read -r -d '' TEMPLATE <<'EOF-TMPL'
{
    "##IFACE##" : {
        "method" : "##METHOD##",
        "nameservers" : [ ##NAMESERVERS##  ]
        ##ADDITIONAL##
    }
}
EOF-TMPL
for i in "${INITIAL_IFACE_NAMES[@]}"; do
    tmp=$( cat "${TMP_DIR}/${IFACES_FILE_INITIAL}" )
    regex="(iface\s+$i.+?)(iface|\Z)"
    tmp=$(echo "$tmp" | perl -lne 'BEGIN{undef $/;} m/'"$regex"'/gs; print $1')

    method=$( echo "$tmp" | perl -lne 'BEGIN{undef $/;} m/iface\s+\S+\s+\S+\s+(\S+)/gs; print $1' )
    address=$( echo "$tmp" | perl -lne 'BEGIN{undef $/;} m/address\s+(\S+)/gs; print $1' )
    netmask=$( echo "$tmp" | perl -lne 'BEGIN{undef $/;} m/netmask\s+(\S+)/gs; print $1' )
    gateway=$( echo "$tmp" | perl -lne 'BEGIN{undef $/;} m/gateway\s+(\S+)/gs; print $1' )

    additional=
    [ -n "$address" ] && additional="${additional}, \"address\" : \"$address\""
    [ -n "$netmask" ] && additional="${additional}, \"netmask\" : \"$netmask\""
    [ -n "$gateway" ] && additional="${additional}, \"gateway\" : \"$gateway\""

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

    template="$TEMPLATE"
    template=${template/\#\#IFACE\#\#/$i}
    template=${template/\#\#METHOD\#\#/$method}
    template=${template/\#\#NAMESERVERS\#\#/$nameservers}
    template=${template/\#\#ADDITIONAL\#\#/$additional}
    echo "$template" > "${TMP_DIR}/${JSON_EXPECTED_FILE}"
  
    simple_get_json_code "${REST_NETCFG}/${i}" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
    echo "$tmp" > "${TMP_DIR}/${JSON_RECEIVED_FILE}"
    bash ./cmpjson.sh -f "${TMP_DIR}/${JSON_RECEIVED_FILE}" "${TMP_DIR}/${JSON_EXPECTED_FILE}" || \
        die "Test case '$TEST_CASE' failed. Expected and returned json do not match."
    HTTP_EXPECTED=200
    [[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

done
echo "SUCCESS"

#######################
### Netcfg_Read_002 ###
#######################
TEST_CASE="Netcfg_Read_002"
echo -e "=====\t\tTEST_CASE: $TEST_CASE\t\t====="

bad_name=$(mktemp -u XXXX)
bad_name="${bad_name}1"

# Request non-existing interface
# Implementation returns 404 or 400 depending on the iface name
simple_get_json_code "${REST_NETCFG}/${bad_name}" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[ $HTTP_CODE -eq 404 -o  $HTTP_CODE -eq 400 ] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 404 or 400, received: $HTTP_CODE."

# api/v1/admin/netcfg/<non_existing_interface>?<junk>
simple_get_json_code "${REST_NETCFG}/${bad_name}?gnsfd=f23sfd+sf4fw4=\?" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[ $HTTP_CODE -eq 404 -o $HTTP_CODE -eq 400 ] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: 404 or 400, received: $HTTP_CODE."
# TODO: Remove any existing interface and the request it. 

# api/v1/admin/netcfg/<junk_here>
HTTP_EXPECTED=400
simple_get_json_code "${REST_NETCFG}/fas40+_245625%20_=345" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

simple_get_json_code "${REST_NETCFG}/40245625%20345f4f34" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

simple_get_json_code "${REST_NETCFG}/asdf%2520%2Bsdf+%25" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

simple_get_json_code "${REST_NETCFG}/eth\ 0\/asdf%2520%2Bsdf+%25" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

# api/v1/admin/netcfg/<existing_interface>?<junk>
last_name="${INITIAL_IFACE_NAMES[-1]}"

HTTP_EXPECTED=200
simple_get_json_code "${REST_NETCFG}/${last_name}?asdf%2520%2Bsdf+%25=20'" tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."


simple_get_json_code "${REST_NETCFG}/${last_name}?asdf%2520%2Bsdf+%25=20+ \ " tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

simple_get_json_code "${REST_NETCFG}/${last_name}?asdf%2520%2Bsdf\ \++%25i \ " tmp HTTP_CODE || die "'api_get_json ${REST_NETCFGS}' failed."
[[ $HTTP_CODE -eq $HTTP_EXPECTED ]] || die "Test case '$TEST_CASE' failed. Expected HTTP return code: $HTTP_EXPECTED, received: $HTTP_CODE."

echo "SUCCESS"

#########################
### Netcfg_Update_001 ###
#########################
# This and the other PUT test are currently ommited due to implementation issues and possible licensing conflict
# TODO revert back
