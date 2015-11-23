#!/bin/bash
#
# Copyright (C) 2015 Eaton
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
#! \file   biostimer-graphs-prefetch.sh
#  \brief  Speed up power-average graphs displaying in the Web UI
#  \author Karol Hrdina <KarolHrdina@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#
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
TYPE_AVG=arithmetic_mean
TYPE_MIN=min
TYPE_MAX=max

TYPES_SUPPORTED="$TYPE_AVG $TYPE_MIN $TYPE_MAX"
STEPS_SUPPORTED="15m 30m 1h 8h 24h"

[ -z "$TYPES" ] && TYPES="$TYPES_SUPPORTED"
[ -z "$STEPS" ] && STEPS="$STEPS_SUPPORTED"

# Regexp wrapped into "^(...)$" when used, so it must describe the whole string
[ -z "$SOURCES_ALLOWED" ] && SOURCES_ALLOWED=""

[ -z "$SUT_HOST" ] && \
        SUT_HOST="127.0.0.1"

[ -z "$SUT_WEB_PORT" ] && \
if [ -n "$CHECKOUTDIR" ] ; then
        SUT_WEB_PORT="8000"
else
        SUT_WEB_PORT="80"
fi

# How many requests can fire at once? Throttle when we reach this amount...
[ -z "$MAX_CHILDREN" ] && MAX_CHILDREN=1

[ -n "$SCRIPTDIR" -a -d "$SCRIPTDIR" ] || \
        SCRIPTDIR="$(cd "`dirname "$0"`" && pwd)" || \
        SCRIPTDIR="`pwd`/`dirname "$0"`" || \
        SCRIPTDIR="`dirname "$0"`"

# Include our standard routines for CI scripts
CI_DEBUG_CALLER="$CI_DEBUG" # may be empty
BASE_URL_CALLER="$BASE_URL" # may be empty
[ -s "$SCRIPTDIR/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/scriptlib.sh || \
{ [ -s "$SCRIPTDIR/../tests/CI/scriptlib.sh" ] &&
        . "$SCRIPTDIR"/../tests/CI/scriptlib.sh ; } || \
{ echo "FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_CHECKOUTDIR=no NEED_BUILDSUBDIR=no determineDirs_default || true
LOGMSG_PREFIX="BIOS-TIMER-"

FETCHER=
( which wget >/dev/null 2>&1 ) && FETCHER=fetch_wget
( which curl >/dev/null 2>&1 ) && FETCHER=fetch_curl

check_in_list() {
    # Tests that tokens listed in "$1" are among
    # the other "$*" space-separated parameters
    local HAYSTACK="$1"
    shift
    for HAY in $HAYSTACK ; do
    for NEEDLE in $* ; do
        [ x"$NEEDLE" = x"$HAY" ] && return 0
    done
    done
    return 1
}

# Different CLI parameters may be added later on...
usage() {
    echo "Usage: $0 [--opt 'arg'] [-j N] [-n | -v | -w]"
    echo "  -n    Dry-run (outputs strings that would be executed otherwise)"
    echo "  -v    Wet-run with output posted to stdout"
    echo "  -w    (default) If not dry-running, actually wet-run the $FETCHER"
    echo "        callouts with results quietly dumped to /dev/null"
    echo "  -q    Suppress the debugging level to 0, disregarding defaults and envvars"
    echo "  -d    Bump up the debugging level to 99, disregarding defaults and envvars"
    echo "  -j N                Max parallel fetchers to launch (default $MAX_CHILDREN)"
    echo "  --lockfile file     Filename used to block against multiple runs"
    echo "  --timefile file     Filename used to save last request end-timestamp"
    echo "  --host hostname     Where to place the REST API request"
    echo "  --port portnum         (default http://$SUT_HOST:$SUT_WEB_PORT)"
    echo "  --steps '15m 24h'   A space-separated string of (supported!) time steps"
    echo "                         (among '$STEPS_SUPPORTED')"
    echo "  --types 'min max'   A space-separated string of (supported!) precalc types"
    echo "                         (among '$TYPES_SUPPORTED')"
    echo "  --src-allow 'regex' A filter for allowed data sources (used if not empty)"
    echo "  --STA 'step:type:allow'  Single argument for one step, type and regex"
    echo "                      these parts may be empty; will also generate"
    echo "                      lockfile and timefile strings to match"
}

[ -z "$FETCHER" ] && \
        logmsg_error "WARNING: Neither curl nor wget were found, wet-run mode would fail"
[ -z "$FETCHER" ] && \
        FETCHER=curl

ACTION="request-quiet"
while [ $# -gt 0 ]; do
    case "$1" in
        -n) ACTION="generate" ;;
        -v) ACTION="request-verbose" ;;
        -w|"") ACTION="request-quiet" ;;
        -j) [ "$2" -ge 1 ] 2>/dev/null || die "Invalid number: '$2'"
            MAX_CHILDREN="$2"
            shift ;;
        --lockfile) LOCKFILE="$2"; shift ;;
        --timefile) TIMEFILE="$2"; shift ;;
        --host|--sut-web-host|--sut-host)
            SUT_HOST="$2"; shift ;;
        --port|--sut-web-port|--sut-port)
            SUT_WEB_PORT="$2"; shift ;;
        # No sanity check enforced against STEPS_SUPPORTED and TYPES_SUPPORTED
        # at the moment, to allow testing of other values as well. However, we
        # do test and issue a sanity-check note below. Can become fatal later.
        --types) TYPES="$2"; shift ;;
        --steps) STEPS="$2"; shift ;;
        --src-allow) SOURCES_ALLOWED="$2"; shift ;;
        --STA) # Parse the single-argument input: Steps:Types:Allowed
            S="`echo "$2" | { IFS=: read _S _T _A; echo "$_S"; }`"
            T="`echo "$2" | { IFS=: read _S _T _A; echo "$_T"; }`"
            A="`echo "$2" | { IFS=: read _S _T _A; echo "$_A"; }`"
            [ -n "$S" ] && STEPS="$S"
            [ -n "$T" ] && TYPES="$T"
            SOURCES_ALLOWED="`echo -e "$A" | sed -e 's,\\\\x2a,\\*,g' -e 's,\\\\x2b,\\+,g' -e 's,\\\\x2e,\\.,g' -e 's,\\\\x7b,\\{,g' -e 's,\\\\x7d,\\},g' -e 's,\\\\x28,\\(,g' -e 's,\\\\x29,\\),g' -e 's,\\\\x3f,\\?,g' -e 's,\\\\x5b,\\[,g' -e 's,\\\\x5d,\\],g' -e 's/\\\\x2c/,/g'`"
            TAG="__`echo -e "$SOURCES_ALLOWED" | sed 's,[\*\?\^\$\(\)\[\]{\}\/\\]*,,g'`@`echo "$S" | sed 's, ,_,g'`"
            [ -n "$T" ] && TAG="$TAG`echo ":$T" | sed 's, ,_,g'`"
            [ -z "$LOCKFILE" ] && \
                LOCKFILE=/var/run/agent-cm/biostimer-graphs-prefetch${TAG}.lock
            [ -z "$TIMEFILE" ] && \
                TIMEFILE=/var/lib/bios/agent-cm/biostimer-graphs-prefetch${TAG}.time
            unset S T A TAG
            shift ;;
        -d) CI_DEBUG=99 ; CI_DEBUG_CALLER=99 ;;
        -q) CI_DEBUG=0 ; CI_DEBUG_CALLER=0 ;;
        -h|--help)
            usage; exit 1 ;;
        *) die "Unknown param(s) follow: '$*'
`usage`" ;;
    esac
    shift
done

# TODO: This may become a die() check later
check_in_list "$TYPES" "$TYPES_SUPPORTED" || \
    logmsg_warn "Invalid TYPES requested (a value in '$TYPES' is not among known '$TYPES_SUPPORTED')"
check_in_list "$STEPS" "$STEPS_SUPPORTED" || \
    logmsg_warn "Invalid STEPS requested (a value in '$STEPS' is not among known '$STEPS_SUPPORTED')"

# TODO: Set paths via standard autoconf prefix-vars and .in conversions
# TODO: Run as non-root, and use paths writable by that user (bios?)
[ -z "$LOCKFILE" ] && \
    LOCKFILE=/var/run/agent-cm/biostimer-graphs-prefetch.lock
[ -z "$TIMEFILE" ] && \
    TIMEFILE=/var/lib/bios/agent-cm/biostimer-graphs-prefetch.time

BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
[ -n "$BASE_URL_CALLER" ] && BASE_URL="$BASE_URL_CALLER" && \
    logmsg_info "Using BASE_URL='$BASE_URL' passed to us via envvars"

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
        for __step in $STEPS; do
            __item=${__item%%_$__step}
        done

        local __type
        for __type in $TYPES; do
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

trap_exit() {
    # Wrapped by settraps, so we do not have nor pass the exit code
    [ -n "$FIRED_BATCH" ] && \
        logmsg_debug "Killing fired fetchers: $FIRED_BATCH" && \
        kill -SIGTERM $FIRED_BATCH 2>/dev/null

    rm -f "$LOCKFILE"
}

trap_abort() {
    logmsg_error 0 "Script aborted by external circumstances!"
    [ -n "$COUNT_TOTAL" ] && [ "$COUNT_TOTAL" != 0 ] && \
        logmsg_info 0 "`date`: Issued $COUNT_TOTAL overall requests (of them $COUNT_SUCCESS successful) by now" >&2
    trap_exit
}

start_lock() {
    # Previous biostimer-graphs-prefetch.sh should execute successfully
    # TODO: see flock command
    [ -f "$LOCKFILE" ] && \
        die 0 "A copy of the script seems already running:" "`cat "$LOCKFILE"`"

    # Set a default handler for default signal list, and then a custom one
    settraps 'trap_abort'
    TRAP_SIGNALS="EXIT" settraps 'trap_exit'

    mkdir -p "`dirname "$LOCKFILE"`" "`dirname "$TIMEFILE"`" && \
    echo "$$" > "$LOCKFILE" || exit $?
}

# Ultimate values that we use in script
END_TIMESTAMP=""
START_TIMESTAMP=""
prepare_timestamps() {
    [ -n "$END_TIMESTAMP" ] && [ -n "$START_TIMESTAMP" ] && return 0

    [ -n "$end_timestamp" ] && \
        logmsg_debug "Using caller-provided end_timestamp='$end_timestamp'" || \
        end_timestamp=$(date -u +%Y%m%d%H%M%SZ)
    END_TIMESTAMP="${end_timestamp}"

    start_timestamp=""
    if [ -n "$TIMEFILE" ] && [ -s "$TIMEFILE" ]; then
        firstline="`head -1 "$TIMEFILE"`"
        if [ -n "$firstline" ]; then
            # TODO: check format of the read line
            start_timestamp="$firstline"
        fi
    fi
    if [ -z "$start_timestamp" ]; then
        start_timestamp="19900101000000Z"
    fi

    START_TIMESTAMP="$start_timestamp"

    export START_TIMESTAMP END_TIMESTAMP
}

generate_getrestapi_strings() {
    generate_getrestapi_strings_sources
    generate_getrestapi_strings_temphum
}

generate_getrestapi_strings_sources() {
    # This is a read-only operation
    local NUM_STRINGS
    NUM_STRINGS=0

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
            if [ -n "$SOURCES_ALLOWED" ] && \
                echo "$s" | egrep -vi "^($SOURCES_ALLOWED$)" > /dev/null ; then
                    logmsg_debug "Source type '$s' did not match SOURCES_ALLOWED filter, skipped" >&2
                    continue
            fi
            for stype in $TYPES; do
                for sstep in $STEPS; do
                    # TODO: change this to api_get with related checks?
                    echo "$FETCHER '$BASE_URL/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${stype}&step=${sstep}&element_id=${i}&source=$s'"
                    NUM_STRINGS=$(($NUM_STRINGS+1))
                    # TODO: Does it make sense to check for 404/500? What can we do?
                done
            done
        done
    done

    logmsg_info "Successfully generated $NUM_STRINGS URLs for power sources" >&2
    return 0
}

generate_getrestapi_strings_temphum() {
    # This is a read-only operation
    # Generate temperature and humidity averages for 4*TH ports of this box
    local NUM_STRINGS
    NUM_STRINGS=0

    stype="$TYPES_SUPPORTED"
    sstep="24h"
    hostname=$(hostname | tr [:lower:] [:upper:])
    i=$(do_select "SELECT id_asset_element FROM t_bios_asset_element WHERE name = '${hostname}'")
    if [ $? = 0 ] && [ -n "$i" ] ; then
        for source in "temperature.TH" "humidity.TH"; do
             for thi in $(seq 1 4); do
                for t in $stype; do
                    for sx in $sstep; do
                s=${source}${thi}
                echo "$FETCHER '$BASE_URL/metric/computed/average?start_ts=${START_TIMESTAMP}&end_ts=${END_TIMESTAMP}&type=${t}&step=${sx}&element_id=${i}&source=$s'"
                NUM_STRINGS=$(($NUM_STRINGS+1))
                    done
                done
            done
        done
    else
        logmsg_error "Could not select id_asset_element for host name '${hostname}', skipping T&H"
    fi

    logmsg_info "Successfully generated $NUM_STRINGS URLs for temperature and humidity sensors" >&2
    return 0
}

# Background processes (if any) listed here for killer-trap
FIRED_BATCH=""
run_getrestapi_strings() {
    # This is a write-possible operation that updates timestamp files.
    # Executes a series of fetch-lines from generate_getrestapi_strings()
    # called inside this routine.

    start_lock
    TS_START="`date -u +%s 2>/dev/null`" || TS_START=""

    COUNT_TOTAL=0
    COUNT_SUCCESS=0
    COUNT_BATCH=0
    RES=-1
    if [ "$MAX_CHILDREN" -gt 1 ] 2>/dev/null; then
        run_getrestapi_strings_parallel "$@"
    else
        run_getrestapi_strings_sequential "$@"
    fi < <(generate_getrestapi_strings)
    # Special bash syntax to avoid forking and thus hiding of variables

    TS_ENDED="`date -u +%s 2>/dev/null`" || TS_ENDED=""

    TS_STRING=""
    [ -n "$TS_START" -a -n "$TS_ENDED" ] && \
    [ "$TS_ENDED" -ge "$TS_START" ] 2>/dev/null && \
        TS_STRING=", took $(($TS_ENDED-$TS_START)) seconds"

    logmsg_info 0 "`date`: Issued $COUNT_TOTAL overall requests (of them $COUNT_SUCCESS successful)${TS_STRING}, done now (RES=$RES)" >&2

    # To push the clock forward, we care about success of those runs which
    # actually requested something and succeeded in all requests
    [ -n "$TIMEFILE" ] && \
    [ $RES = 0 ] && [ "$COUNT_SUCCESS" -gt 0 ] && \
    [ "$COUNT_SUCCESS" = "$COUNT_TOTAL" ] && \
        echo "$END_TIMESTAMP" > "$TIMEFILE"

    [ $RES -le 0 ] && return 0
    return $RES
}

run_getrestapi_strings_sequential() {
    # Internal detail for run_getrestapi_strings()
    logmsg_info 0 "`date`: Using sequential foreground REST API requests" >&2
    while IFS="" read CMDLINE; do
        [ "$RES" -lt 0 ] && RES=0
        COUNT_TOTAL=$(($COUNT_TOTAL+1))
        eval $CMDLINE
        RESLINE=$?
        [ "$1" = -v ] && logmsg_debug 0 "Result ($RESLINE) of: $CMDLINE" >&2
        [ "$RESLINE" = 0 ] && \
             COUNT_SUCCESS=$(($COUNT_SUCCESS+1)) || \
             RES=$RESLINE
    done
}

run_getrestapi_strings_parallel() {
    # Internal detail for run_getrestapi_strings()
    logmsg_info 0 "`date`: Using parallel background REST API requests (at most $MAX_CHILDREN at once)" >&2
    while IFS="" read CMDLINE; do
        [ "$RES" -lt 0 ] && RES=0
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
    done

    for F in $FIRED_BATCH ; do
        wait $F
        RESLINE=$?
        [ "$RESLINE" = 0 ] && \
             COUNT_SUCCESS=$(($COUNT_SUCCESS+1)) || \
             RES=$RESLINE
    done
    FIRED_BATCH=""
}

### script starts here ###
prepare_timestamps

logmsg_info 0 "`date`: ${_SCRIPT_PATH} ${_SCRIPT_ARGS}
Active settings:
  ACTION        $ACTION
  LOCKFILE      $LOCKFILE
  TIMEFILE      $TIMEFILE
  STEPS         $STEPS
  TYPES         $TYPES
  CI_DEBUG      $CI_DEBUG
  MAX_CHILDREN  $MAX_CHILDREN
  FETCHER       $FETCHER
  SOURCES_ALLOWED '$SOURCES_ALLOWED'
  START_TIMESTAMP $START_TIMESTAMP
  END_TIMESTAMP   $END_TIMESTAMP
" >&2

case "$ACTION" in
    generate)
        # Moderate logging - ERROR and WARN - by default
        [ x"$CI_DEBUG_CALLER" = x ] && CI_DEBUG=2
        [ -z "$FETCHER" ] && \
                FETCHER=curl
        generate_getrestapi_strings "$@"
        exit $?
        ;;
    request-verbose) # production run with verbose output
        [ x"$CI_DEBUG_CALLER" = x ] && CI_DEBUG=5
        [ -z "$FETCHER" ] && \
                die "No usable FETCHER was detected on this system"
        run_getrestapi_strings "$@"
        exit $?
        ;;
    request-quiet) # default / quiet mode for timed runs
        # If user did not ask for debug shut it:
        [ x"$CI_DEBUG_CALLER" = x ] && CI_DEBUG=0
        [ -z "$FETCHER" ] && \
                die "No usable FETCHER was detected on this system"
        run_getrestapi_strings "$@" >/dev/null
        exit $?
        ;;
esac

# Should not get here
exit 1
