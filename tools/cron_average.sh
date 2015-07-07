#!/bin/bash

# USAGE
# cron_average 
# At the moment script only outputs curl requests

# PURPOSE:
# Have cron job periodically call this script 

# TODO
#   simple lockfile preventing unintentional double runs
#   store value of last run in lockfile, otherwise assign start_timestamp value of 0 (== 1970-01-01) 
#   Add <start_timestamp> <end_timestamp> AND
#   convert `date +%s`'s format to rfc-11 format so that caller does not have to bother with this "knowledge"


declare -r START_TIMESTAMP=
declare -r END_TIMESTAMP=

declare -ar TYPE=("arithmetic_mean" "min" "max")
declare -ar STEP=("15m" "30m" "1h" "8h" "24h")

declare -r SERVER="127.0.0.1"
declare -r PORT="8000"

# Helper echo to stderr function
echoerr() { echo "$@" 1>&2; }

# Helper join array function
function join { local IFS="$1"; shift; echo "$*"; }

# Converts asset element id to (discovered) device element id
# Returns 1 on error
# Arguments:
# $1 - asset element id
# $2 - name of output parameter
element_to_device_id () {
    if [ $# -ne 2 ]; then
        echoerr "Function element_to_device_id() requires two arguments; 1. element_id 2. name of output variable."
        return 2
    fi
    if [ -z "$1" -o -z "$2" ]; then
        echoerr "One or both of the arguments to element_to_device_id() is empty."
        return 2
    fi
    if [ "$1" == "0" ]; then
        echoerr "First argument to element_to_device_id() is '0' which is not valid asset element identifier."
        return 2
    fi
    local __element_id="$1"
    local __resultvar="$2"
    
    local __result=$(mysql -s -u root -D box_utf8 -N -e "    
        SELECT a.id_discovered_device
        FROM
            t_bios_discovered_device AS a LEFT JOIN t_bios_monitor_asset_relation AS b
            ON a.id_discovered_device = b.id_discovered_device
        WHERE
            id_asset_element = '${__element_id}'
        ")
    if [ -z "$__result" ]; then
        return 1
    else
        eval $__resultvar="'$__result'" #Note: This is evil, i know...
        return 0
    fi    
}

# Retrieve asset element id's for datacenter, rack, ups. Output parameter contains whitespace delimited id's.
#   
# Returns 1 on error
# Arguments:
# $1 - name of output parameter
get_elements () {
    if [ $# != 1 ]; then
        echoerr "Function get_elements() requires one argument; 1. name of output variable."
        return 2
    fi
    if [ -z "$1" ]; then
        echoerr "Arguments to is empty."
        return 2
    fi

    local __resultvar="$1"
    local __result=$(mysql -s -u root -D box_utf8 -N -e "    
        SELECT id FROM v_web_element
        WHERE
            id_type = 
            (
                SELECT id_asset_element_type
                FROM t_bios_asset_element_type
                WHERE name = 'datacenter'
            ) OR
            id_type = 
            (
                SELECT id_asset_element_type
                FROM t_bios_asset_element_type
                WHERE name = 'rack'
            ) OR
            (
                id_type =
                (
                    SELECT id_asset_element_type
                    FROM t_bios_asset_element_type
                    WHERE name = 'device'

                ) AND
                subtype_id = 
                (
                    SELECT id_asset_device_type
                    FROM t_bios_asset_device_type
                    WHERE name = 'ups'
                )
            )
            ")
    if [ -z "$__result" ]; then
        return 1
    else
        eval $__resultvar="'$__result'" #Note: This is evil, i know...
        return 0
    fi
}

# Retrieve sources of a specific device id. Output parameter contains whitespace delimited id's.
#   
# Returns 1 on error
# Arguments:
# $1 - device id
# $2 - name of output parameter
# TODO: rename some local variables to stay consistent with __var
sources_from_device_id () {
    if [ $# -ne 2 ]; then
        echoerr "Function sources_from_device_id() requires two arguments; 1. device id 2. name of output variable."
        return 2
    fi
    if [ -z "$1" -o -z "$2" ]; then
        echoerr "One or both of the arguments to sources_from_device_id() is empty."
        return 2
    fi
    if [ "$1" == "0" ]; then
        echoerr "First argument to sources_from_device_id() is '0' which is not valid asset device identifier."
        return 2
    fi

    local __device_id="$1"
    local __resultvar="$2"
    local __result=$(mysql -s -u root -D box_utf8 -N -e "    
        SELECT topic
        FROM v_bios_measurement_topic
        WHERE device_id = '${__device_id}'
    ")

    if [ -z "$__result" ]; then
        return 1
    fi

    # all arrays are local by default
    declare -a __sources
    declare -a __tmp

    read -a __tmp <<<$__result
    local item=
    local template=
    for item in "${__tmp[@]}"; do
        item=${item%%@*}

        local sstep=
        for sstep in ${STEP[@]}; do
            template="_$sstep"
            item=${item%%$template}
        done

        local stype=
        for stype in ${TYPE[@]}; do
            template="_$stype"
            item=${item%%$template}
        done

        local already_inserted=0
        local i=
        for i in "${__sources[@]}"; do
            if [ "$item" == "$i" ]; then
                already_inserted=1
            fi
        done
        if [ $already_inserted -eq 0 ]; then
          __sources+=($item)
        fi
    done

    __result=$(join ' ' ${__sources[@]})
    eval $__resultvar="'$__result'" #Note: This is evil, i know...
    return 0
}

get_elements element_ids
if [ $? -eq 1 ]; then
    # No element id's for DC, rack, ups
    exit 0
elif [ $? -gt 1 ]; then
    exit 1
fi
read -a ELEMENTS <<<${element_ids}

#TODO: better variable names
for i in "${ELEMENTS[@]}"; do
    echo "-> $i"
    device_id=
    element_to_device_id $i device_id 
    if [ $? -eq 1 ]; then
        # element_to_device_id() not found for this one
        continue
    elif [ $? -gt 1 ]; then
        exit 1
    fi
    echo "$i"
    echo "device id: '$device_id'"
    echo -e "\tsources:"
    sources=    
#set -x
    sources_from_device_id $device_id sources
#set +x
    if [ $? -eq 1 ]; then
        continue
    elif [ $? -gt 1 ]; then
        exit 1
    fi
    read -a SOURCES <<<${sources}
    for s in ${SOURCES[@]}; do
        for stype in ${TYPE[@]}; do
            for sstep in ${STEP[@]}; do
                echo "curl 'http://${SERVER}:${PORT}/api/v1/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${stype}&step=${sstep}&element_id=${i}&source=$s'"
            done
        done
    done

done


