#!/usr/bin/env bash
#
#   Copyright (c) 2014 - 2020 Eaton
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    cmpjson.sh
#  \brief   Compare REST API call outputs with expected ones
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Compare REST API call outputs with expected ones - it assumes
#           one normalized JSON document per line to support more results
#           in one file! That is, file1:lineN is compared to file2:lineN.
#  \note    Most of the logic is in separate tools/JSON.sh script!
#
# Usage: bash cmpjson.sh file1 file2

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
determineDirs || true

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh" "$SCRIPTDIR/../../tools/JSON.sh" "/usr/share/fty/scripts/JSON.sh" "/usr/share/bios/scripts/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

# By default we do sorted comparisons; pass a " " space envvar to unset options
[ -z "${JSONSH_OPTIONS-}" ] && JSONSH_OPTIONS="-N=-n -Nnx=%.16f"
[ -z "${JSONSH_OPTIONS_VERBOSE-}" ] && JSONSH_OPTIONS_VERBOSE="-S=-n -Nnx=%.16f"

[ -n "$JSONSH" ] && [ -x "$JSONSH" ] || \
    die "JSON.sh is not executable (tried '${JSONSH-}')"

[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || \
if [ -n "${BASH-}" ] && . "$JSONSH" ; then
    logmsg_debug "cmpjson: Will use sourced JSON.sh from '$JSONSH'"
else
    logmsg_debug "cmpjson: Will fork to use JSON.sh from '$JSONSH'"
    jsonsh_cli() { "$JSONSH" "$@"; }
fi
JSONSH_CLI_DEFINED=yes

self_test() {
    local jsonstr1='{"current":[{"id":3,"realpower.1":1,"voltage.2":1,"current.2":12,"current.1":31,"voltage.1":3}]}'
    local jsonstr2='{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.1":3}]}'
    local jsonstr3='{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.2":3}]}'

    logmsg_info "=== This test should show no differences if JSON content sorting is enabled:"
    cmpjson_strings "$jsonstr1" "$jsonstr2" || die "json1 should equal to json2 (when sorted)"

    logmsg_info "=== This test should find some differences:"
    cmpjson_strings "$jsonstr1" "$jsonstr3" && die "json1 should NOT equal to json3"

    :
}

cmpjson_strings() {
    normstr1="`echo "$1" | eval jsonsh_cli $JSONSH_OPTIONS`"
    res1=$?
    normstr2="`echo "$2" | eval jsonsh_cli $JSONSH_OPTIONS`"
    res2=$?
    # If some parsing errored out, it was reported above; fall through to error
    if [ "$res1" = 0 -a "$res2" = 0 ]; then
        if [ x"$normstr1" = x"$normstr2" ]; then
            return 0
        else
            TMPF1="/tmp/.cmpjson-$$-tmpf1"
            TMPF2="/tmp/.cmpjson-$$-tmpf2"
            rm -f "$TMPF1" "$TMPF1"
            touch "$TMPF1" "$TMPF1" && \
            chmod 600 "$TMPF1" "$TMPF1" && \
            settraps "rm -f '$TMPF1' '$TMPF2'" && \
            { echo "$1" | eval jsonsh_cli -l $JSONSH_OPTIONS_VERBOSE > "$TMPF1"; res1=$?
              echo "$2" | eval jsonsh_cli -l $JSONSH_OPTIONS_VERBOSE > "$TMPF2"; res2=$?
              [ "$res1" = 0 -a "$res2" = 0 ] && diff -bu "$TMPF1" "$TMPF2"; }
            rm -f "$TMPF1" "$TMPF2"
            settraps '-'
        fi
    fi

    [ "$res1" != 0 ] && logmsg_error "Error parsing input 1:" && \
        echo "$1" >&2
    [ "$res2" != 0 ] && logmsg_error "Error parsing input 2:" && \
        echo "$2" >&2

    return 1
}

cmpjson_files() {
    # Note: our results are not valid json documents, but each line contains
    # one - thus return a list of object for each line
    # Parallel file reading code below inspired by article and comments(!) at
    # http://www.linuxjournal.com/content/reading-multiple-files-bash
    local FD1=7
    local FD2=8
    local file1="$1"
    local file2="$2"
    local count1=0
    local count2=0
    local eof1=0
    local eof2=0
    local data1
    local data2
    local RES=255
    # Open files
    eval exec "$FD1<'$file1'" || return
    eval exec "$FD2<'$file2'" || return
    while [ "$eof1" = 0 -o "$eof2" = 0 ]
    do
        if read data1 <&$FD1; then
            let count1++
            # printf "%s, line %d: %s\n" $file1 $count1 "$data1" >&2
        else
            eof1=1
        fi

        if read data2 <&$FD2; then
            let count2++
            # printf "%s, line %d: %s\n" $file2 $count2 "$data2" >$2
        else
            eof2=1
        fi

        # Both empty files - ok, contents are the same
        [ "$eof1" = 1 -a "$eof2" = 1 -a "$RES" = 255 ] && RES=0

        if [ "$eof1" = 0 -a "$eof2" = 0 ] ; then
            [ "$RES" = 255 ] && RES=0
            cmpjson_strings "$data1" "$data2"
            if [ $? != 0 ]; then
                RES=$(($RES+1))
                logmsg_error "^^^ Above we FAILED comparison of lines number $count1($count2) in the source JSON multi-docs" >&2
            fi
        fi
    done
    # Close files
    eval exec "$FD1>&-"
    eval exec "$FD2>&-"
    if [ "$RES" = 0 ] ; then
        if  [ "$count1" = 0 -a "$count2" != 0 ] || \
            [ "$count2" = 0 -a "$count1" != 0 ]; then RES=255; fi
    fi
    [ "$RES" = 255 ] && \
        logmsg_error "One of the files '$file1' or '$file2' is empty" >&2
    if [ "$eof1" = 0 -o "$eof2" = 0 -o "$count1" != "$count2" ]; then
        logmsg_error "Read $count1 lines from '$file1' EOF1=$eof1 but $count2 lines from '$file2' EOF2=$eof2" >&2
        [ "$RES" = 0 ] && RES=126
    fi
    [ $RES != 0 ] && \
        logmsg_error "Files '$file1' and '$file2' do not contain equivalent JSON content" >&2
    return $RES
}

usage() {
        echo "Usage: $0 {file1} {file2}"
        echo "  The two files should contain the same amount of single-line JSON documents"
        echo "Usage: $0 -f {file1} {file2}"
        echo "  Each of two files should contain a complete JSON document, maybe multiline"
        echo "Usage: $0 -s {string1} {string2}"
        echo "  The two strings should each contain a complete JSON document"
        echo "Usage: $0 -t"
        echo "  Self-testing with pre-defined strings"
        echo "This script accepts the following envvars:"
        echo "  JSONSH (current: $JSONSH) - Path to JSON.sh script"
        echo "  JSONSH_OPTIONS (current: $JSONSH_OPTIONS) - its options for output"
        echo "  JSONSH_OPTIONS_VERBOSE (current: $JSONSH_OPTIONS_VERBOSE)" \
            "- its options for verbose output (diff of mismatches)"
        echo ""
}

case "$1" in
    -t)
        self_test
        exit $?
        ;;
    -f) [ ! -r "$2" -o ! -r "$3" ] && usage && \
            die "Not readable files '$2' and '$3' were provided!"

        cmpjson_strings "`cat "$2"`" "`cat "$3"`"
        exit $?
        ;;
    -s) cmpjson_strings "$2" "$3"; exit ;;
    -h|--help)
        usage
        exit 0
        ;;
esac

[ $# != 2 ] && usage && die "Bad number of parameters ($#)!"
[ ! -r "$1" -o ! -r "$2" ] && usage && \
    die "Not readable files '$1' and '$2' were provided!"

cmpjson_files "$1" "$2"
exit $?
