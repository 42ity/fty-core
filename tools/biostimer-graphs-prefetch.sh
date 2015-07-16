#!/bin/bash

# USAGE
# biostimer-graphs-prefetch.sh
# Script finds assets with power-average data and triggers pre-calculation
# of the averages needed for graphs, thus speeding them up in web-GUI.
# At the moment script only outputs curl requests in dry-run mode (-n)
# and can execute them quietly (-q/default) or verbosely (-v).
#
# PURPOSE:
# Have cron job periodically call this script

# supported average types and steps
declare -a TYPE=("arithmetic_mean" "min" "max")
declare -a STEP=("15m" "30m" "1h" "8h" "24h")

[ -z "$SUT_HOST" ] && \
declare -r SUT_HOST="127.0.0.1"

[ -z "$SUT_WEB_PORT" ] && \
if [ -n "$CHECKOUTDIR" ] ; then
        declare SUT_WEB_PORT="8000"
else
        declare SUT_WEB_PORT="80"
fi

[ -z "$BASE_URL" ] && BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"

# How many requests can fire at once? Throttle when we reach this amount...
[ -z "$MAX_CHILDREN" ] && MAX_CHILDREN=32

[ -n "$SCRIPTDIR" -a -d "$SCRIPTDIR" ] || \
        SCRIPTDIR="$(cd "`dirname "$0"`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname "$0"`" || \
        SCRIPTDIR="`dirname "$0"`"

# Include our standard routines for CI scripts
CI_DEBUG_CALLER="$CI_DEBUG" # may be empty
[ -s "$SCRIPTDIR/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/scriptlib.sh || \
{ [ -s "$SCRIPTDIR/../tests/CI/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/../tests/CI/scriptlib.sh ; } || \
{ echo "FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_CHECKOUTDIR=no NEED_BUILDSUBDIR=no determineDirs_default || true
LOGMSG_PREFIX="BIOS-TIMER-"

# TODO: Set paths via standard autoconf prefix-vars and .in conversions
# TODO: Run as non-root, and use paths writable by that user (bios?)
LOCKFILE=/var/run/biostimer-graphs-prefetch.lock
TIMEFILE=/var/lib/bios/agent-cm/biostimer-graphs-prefetch.time

FETCHER=
( which wget >/dev/null 2>&1 ) && FETCHER=fetch_wget
( which curl >/dev/null 2>&1 ) && FETCHER=fetch_curl

include_cfg() {
    [ -s "$1" ] && [ -r "$1" ] && \
        logmsg_info "Using configuration file '$1'..." && \
        . "$1"
}

for F in /etc/bios/biostimer-graphs-prefetch.conf \
         /etc/bios/biostimer-graphs-prefetch.conf.local \
; do include_cfg "$F"; done

[ -z "$FETCHER" ] && \
        echo "WARNING: Neither curl nor wget were found, wet-run mode would fail" && \
        FETCHER=curl

# Different CLI parameters may be added later on...
usage() {
    echo "Usage: $0 [-C file.conf] [-j N] [-n | -v | -q]"
    echo "  -n    Dry-run (outputs strings that would be executed otherwise)"
    echo "  -v    Wet-run with output posted to stdout"
    echo "  -q    (default) If not dry-running, actually run the $FETCHER"
    echo "        callouts with results quietly dumped to /dev/null"
    echo "  -C file     Include a configuration file to override this run"
    echo "  -j N        Max parallel fetchers to launch (default $MAX_CHILDREN)"
    echo "  --lockfile file     Filename used to block against multiple runs"
    echo "  --timefile file     Filename used to save last request end-timestamp"
}

ACTION="generate"
while [ $# -gt 0 ]; do
    case "$1" in
        -n) ACTION="generate" ;;
        -v) ACTION="request-verbose" ;;
        -q|"") ACTION="request-quiet" ;;
        -C) include_cfg "$2" || die 127 "Can not use config file '$2'"
            shift ;;
        -j) [ "$2" -ge 1 ] 2>/dev/null || die "Invalid number: '$2'"
            MAX_CHILDREN="$2"
            shift ;;
        --lockfile) LOCKFILE="$2"; shift ;;
        --timefile) TIMEFILE="$2"; shift ;;
        -h|--help)
            usage; exit 1 ;;
        *) die "Unknown param(s) follow: '$@'
`usage`" ;;
    esac
    shift
done

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
        logmsg_error "element_to_device_id(): argument required."
        return 2
    fi
    if [ -z "$1" -o "$1" == "0" ]; then
        logmsg_error "element_to_device_id(): argument empty or '0' which is not a valid asset element identifier."
        return 2
    fi

    local __element_id="$1"
    local __result=$(do_select "
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
    local __result=$(do_select "
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
        logmsg_error "sources_from_device_id(): argument required."
        return 2
    fi
    if [ -z "$1" -o "$1" == "0" ]; then
        logmsg_error "sources_from_device_id(): argument empty or '0' which is not a valid device identifier."
        return 2
    fi

    local __device_id="$1"
    local __mysql=$(do_select "
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
    # Previous biostimer-graphs-prefetch.sh should execute successfully
    # TODO: see flock command
    [ -f "$LOCKFILE" ] && \
        die 0 "A copy of the script seems already running:" "`cat "$LOCKFILE"`"

    settraps '[ -n "$FIRED_BATCH" ] && logmsg_debug "Killing fired fetchers: $FIRED_BATCH" && kill -SIGTERM $FIRED_BATCH 2>/dev/null; rm -f "$LOCKFILE"'
    mkdir -p "`dirname "$LOCKFILE"`" "`dirname "$TIMEFILE"`" && \
    touch "$LOCKFILE" || exit $?
}

generate_getrestapi_strings() {
    # This is a read-only operation
    [ -n "$end_timestamp" ] && \
        logmsg_debug "Using caller-provided end_timestamp='$end_timestamp'" || \
        end_timestamp=$(date -u +%Y%m%d%H%M%SZ)
    declare -r END_TIMESTAMP="${end_timestamp}"

    start_timestamp=""
    if [ -s "$TIMEFILE" ]; then
        firstline="`head -1 $TIMEFILE`"
        if [ -n "$firstline" ]; then
            # TODO: check format of the read line
            start_timestamp="$firstline"
        fi
    fi
    if [ -z "$start_timestamp" ]; then
        start_timestamp="19900101000000Z"
    fi

    declare -r START_TIMESTAMP="$start_timestamp"

    logmsg_debug $CI_DEBUGLEVEL_DEBUG \
        "START_TIMESTAMP=$START_TIMESTAMP" \
        "END_TIMESTAMP=$END_TIMESTAMP" >&2

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
        logmsg_debug $CI_DEBUGLEVEL_DEBUG "-> $i" >&2

        # 3. Try to get corresponding device id from given element id
        device_id=$(element_to_device_id $i)    
        if [ $? -eq 1 ]; then   # device id not found for this element id
            continue
        elif [ $? -gt 1 ]; then # error
            return 1
        fi

        logmsg_debug $CI_DEBUGLEVEL_DEBUG "$i" "device id: '$device_id'" " sources:" >&2

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
                    # TODO: change this to api_get with related checks?
                    echo "$FETCHER '$BASE_URL/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${stype}&step=${sstep}&element_id=${i}&source=$s'"
                    # TODO: Does it make sense to check for 404/500? What can we do?
                done
            done
        done
    done

    #6 Generate temperature and humidity averages
    stype="${TYPE[0]}"
    sstep="24h"
    hostname=$(hostname | tr [:lower:] [:upper:])
    i=$(do_select "SELECT id_asset_element FROM t_bios_asset_element WHERE name = '${hostname}'")
    if [ $? = 0 ] && [ -n "$i" ] ; then
        for source in "temperature.TH" "humidity.TH"; do
             for thi in $(seq 1 4); do
                s=${source}${thi}
                echo "$FETCHER '$BASE_URL/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${stype}&step=${sstep}&element_id=${i}&source=$s'"
            done
        done
    else
        logmsg_error "Could not select id_asset_element for host name '${hostname}', skipping T&H"
    fi

    return 0
}

run_getrestapi_strings() {
    # This is a write-possible operation that updates timestamp files.
    # Executes a series of fetch-lines from generate_getrestapi_strings()
    # called inside this routine.

    start_lock
    TS_START="`date -u +%s 2>/dev/null`" || TS_START=""

    COUNT_TOTAL=0
    COUNT_SUCCESS=0
    COUNT_BATCH=0
    FIRED_BATCH=""
    RES=-1
    while IFS="" read CMDLINE; do
        COUNT_TOTAL=$(($COUNT_TOTAL+1))
        COUNT_BATCH=$(($COUNT_BATCH+1))
        ( [ "$1" = -v ] && logmsg_debug 0 "Running: $CMDLINE" >&2
          eval $CMDLINE
          RESLINE=$?
          [ "$1" = -v ] && logmsg_debug 0 "Result ($RESLINE) of: $CMDLINE" >&2
          exit $RESLINE
        ) &
        FIRED_BATCH="$FIRED_BATCH $!"
        if [ "$COUNT_BATCH" -ge "$MAX_CHILDREN" ]; then
            [ "$1" = -v ] && logmsg_debug 0 \
                "Reached $COUNT_BATCH requests running at once ($COUNT_TOTAL overall), throttling down..." >&2
            for F in $FIRED_BATCH ; do
                wait $F
                RESLINE=$?
                [ "$RESLINE" = 0 ] && \
                    COUNT_SUCCESS=$(($COUNT_SUCCESS+1)) || \
                    RES=$RESLINE
            done
            COUNT_BATCH=0
            FIRED_BATCH=""
        fi
    done < <(generate_getrestapi_strings)
    # Special bash syntax to avoid forking and thus hiding of variables

    for F in $FIRED_BATCH ; do
        wait $F
        RESLINE=$?
        [ "$RESLINE" = 0 ] && \
             COUNT_SUCCESS=$(($COUNT_SUCCESS+1)) || \
             RES=$RESLINE
    done
    FIRED_BATCH=""

    TS_ENDED="`date -u +%s 2>/dev/null`" || TS_ENDED=""

    TS_STRING=""
    [ -n "$TS_START" -a -n "$TS_ENDED" ] && \
    [ "$TS_ENDED" -ge "$TS_START" ] 2>/dev/null && \
        TS_STRING=", took $(($TS_ENDED-$TS_START)) seconds"

    logmsg_info 0 "`date`: Completed $COUNT_TOTAL overall requests (of them $COUNT_SUCCESS successful)${TS_STRING}, done now" >&2

    [ $RES = 0 ] && echo "$END_TIMESTAMP" > "$TIMEFILE"
    [ $RES -le 0 ] && return 0
    return $RES
}

### script starts here ###
case "$ACTION" in
    generate)
        generate_getrestapi_strings "$@"
        exit $?
        ;;
    request-verbose) # production run with verbose output
        [ x"$CI_DEBUG_CALLER" = x ] && CI_DEBUG=5
        run_getrestapi_strings "$@"
        exit $?
        ;;
    request-quiet) # default / quiet mode for timed runs
        # If user did not ask for debug shut it:
        [ x"$CI_DEBUG_CALLER" = x ] && CI_DEBUG=0
        run_getrestapi_strings "$@" >/dev/null
        exit $?
        ;;
esac

# Should not get here
exit 1
