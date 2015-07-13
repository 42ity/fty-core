#!/bin/bash

# USAGE
# cron_average 
# At the moment script only outputs curl requests
#
# PURPOSE:
# Have cron job periodically call this script 

# supported average types and steps
declare -ar TYPE=("arithmetic_mean" "min" "max")
declare -ar STEP=("15m" "30m" "1h" "8h" "24h")

declare -r SERVER="127.0.0.1"
declare -r PORT="8000"

# TODO: set path via standard autoconf prefix-vars and .in conversions
LOCKFILE=/var/lib/bios/agent-cm/cron_average.lock
TIMEFILE=/var/lib/bios/agent-cm/cron_average.time

FETCHER=
( which curl 2>/dev/null ) && FETCHER=fetch_curl
( which wget 2>/dev/null ) && FETCHER=fetch_wget

[ -z "$FETCHER" ] && \
        echo "WARNING: Neither curl nor wget were found, wet-run mode would fail" && \
        FETCHER=curl

# TODO: rely on (and include in distro) the scriptlib.sh library
# Helper echo to stderr function
echoerr() { echo "$@" 1>&2; }

# Helper join array function
function join { local IFS="$1"; shift; echo "$*"; }

fetch_wget() {
    wget -q -O - "$@"
}

fetch_curl() {
    curl "$@"
}

# Converts asset element id to (discovered) device element id
#
# Returns:
#   2 - error (bad args, mysql error ...)
#   1 - no device id exists for given asset element
#   0 - success
# Arguments:
#   $1 - asset element id
element_to_device_id() {
    # arguments check
    if [ $# -lt 1 ]; then
        echoerr "element_to_device_id(): argument required."
        return 2
    fi
    if [ -z "$1" -o "$1" == "0" ]; then
        echoerr "element_to_device_id(): argument empty or '0' which is not a valid asset element identifier."
        return 2
    fi

    local __element_id="$1"
    local __result=$(mysql -s -u root -D box_utf8 -N -e "    
        SELECT a.id_discovered_device
        FROM
            t_bios_discovered_device AS a LEFT JOIN t_bios_monitor_asset_relation AS b
            ON a.id_discovered_device = b.id_discovered_device
        WHERE
            id_asset_element = '${__element_id}'
        ")
    if [ $? -ne 0 ]; then
        return 2
    fi
    if [ -z "$__result" ]; then
        return 1
    else
        echo "$__result"
        return 0
    fi
}

# Retrieve asset element id's for datacenter, rack, ups. Output parameter contains whitespace delimited id's.
#
# Returns:
#   2 - error
#   1 - no asset element identifiers of requested type found
#   0 - success
# Arguments:
#   None
get_elements() {
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
    if [ $? -ne 0 ]; then
        return 2
    fi
    if [ -z "$__result" ]; then
        return 1
    else
        echo "$__result"
        return 0
    fi
}

# Retrieve sources of a specific device id. Output parameter contains whitespace delimited id's.
#
# Returns:
#   2 - error
#   1 - on error
#   0 - success
# Arguments:
#   $1 - device identifier
sources_from_device_id() {
    # arguments check
    if [ $# -lt 1 ]; then
        echoerr "sources_from_device_id(): argument required."
        return 2
    fi
    if [ -z "$1" -o "$1" == "0" ]; then
        echoerr "sources_from_device_id(): argument empty or '0' which is not a valid device identifier."
        return 2
    fi

    local __device_id="$1"
    local __mysql=$(mysql -s -u root -D box_utf8 -N -e "
        SELECT topic
        FROM v_bios_measurement_topic
        WHERE device_id = '${__device_id}'
    ")
    if [ $? -ne 0 ]; then
        return 2
    fi
    if [ -z "$__mysql" ]; then
        return 1
    fi

    # all arrays are local by default
    declare -a __sources

    local __item

    for __item in $__mysql; do
        __item=${__item%%@*}

        local __step
        for __step in ${STEP[@]}; do
            __item=${__item%%_$__step}
        done

        local __type
        for __type in ${TYPE[@]}; do
            __item=${__item%%_$__type}
        done

        local __already_inserted=0
        local __i=
        for __i in "${__sources[@]}"; do
            if [ "$__item" == "$__i" ]; then
                __already_inserted=1
                break
            fi
        done
        if [ $__already_inserted -eq 0 ]; then
          __sources+=($__item)
        fi
    done

    local __result=$(join ' ' ${__sources[@]})
    echo "$__result"
    return 0
}

start_lock() {
    # Previous cron_average.sh should execute successfully
    # TODO: see flock command
    [ -f "$LOCKFILE" ] && exit 0

    trap '{ EXITCODE=$?; rm -f "$LOCKFILE" ; exit $EXITCODE; }' EXIT # TODO add other signals
    mkdir -p "`dirname "$LOCKFILE"`" "`dirname "$TIMEFILE"`" && \
    touch "$LOCKFILE" || exit $?
}

generate_curl_strings() {
    end_timestamp=$(date -u +%Y%m%d%H%M%S)
    declare -r END_TIMESTAMP="${end_timestamp}Z"

    start_timestamp=""
    if [ -s "$TIMEFILE" ]; then
        firstline="`head -1 $TIMEFILE`"
        if [ -n "$firstline" ]; then
            # TODO: check format of the read line
            start_timestamp="$firstline"
        fi
    fi
    if [ -z "$start_timestamp" ]; then
        start_timestamp="19700101000000Z"
    fi

    declare -r START_TIMESTAMP="$start_timestamp"

    # TODO: remove or convert to logmsg_debug
    #echo "START_TIMESTAMP=$START_TIMESTAMP"
    #echo "END_TIMESTAMP=$END_TIMESTAMP"

    # 1. Get asset element identifiers
    element_ids=$(get_elements)
    if [ $? -eq 1 ]; then   # no element ids for DC, rack, ups
        return 0
    elif [ $? -gt 1 ]; then # error
        return 1
    fi
    #read -a ELEMENTS <<<${element_ids}

    # 2. Go over them and ...
    for i in $element_ids; do

        # TODO: Remove the following block or convert to logmsg_debug
        #echo "-> $i"

        # 3. Try to get corresponding device id from given element id
        device_id=$(element_to_device_id $i)    
        if [ $? -eq 1 ]; then   # device id not found for this element id
            continue
        elif [ $? -gt 1 ]; then # error
            return 1
        fi

        # TODO: Remove the following block or convert to logmsg_debug
        #echo "$i"
        #echo "device id: '$device_id'"
        #echo -e "\tsources:"

        # 4. Try to get distinct sources from all the topics of a given device id
        SOURCES=$(sources_from_device_id $device_id)
        if [ $? -eq 1 ]; then   # no sources/topics exist for given device id
            continue
        elif [ $? -gt 1 ]; then # error
            return 1
        fi
        # 5. Generate curl request for each combination of source, step, type for the given element id
        for s in $SOURCES; do
            for stype in ${TYPE[@]}; do
                for sstep in ${STEP[@]}; do
                    # TODO: change this to a command instead of echo
                    echo "$FETCHER 'http://${SERVER}:${PORT}/api/v1/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${stype}&step=${sstep}&element_id=${i}&source=$s'"
                    # TODO: Does it make sense to check for 404/500? What can we do?
                done
            done
        done
    done

    return 0
}

### script starts here ###
case "$1" in
    -n) generate_curl_strings
        exit $?
        ;;
    -h|--help) echo "$0 [-n]"
        echo "  -n    Dry-run"
        echo "If not dry-running, actually run the $FETCHER callouts"
        ;;
    "") start_lock
        generate_curl_strings | while IFS="" read LINE; do
            # TODO: logmsg_debug this:
            echo "Running: $LINE"
            $LINE &
        done
        wait
        RES=$?
        [ $RES = 0 ] && echo "$END_TIMESTAMP" > "$TIMEFILE"
        exit $RES
        ;;
    *) echo "Unknown params : $@";;
esac

exit 1
