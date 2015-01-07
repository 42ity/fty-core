#!/usr/bin/env bash

# Compare REST API call outputs with expected ones - it assumes
# one normalized JSON document per line to support more results
# in one file! That is, file1:lineN is compared to file2:lineN.
# NOTE: Most of the logic is in separate tools/JSON.sh script!
# Usage: bash cmpjson.sh file1 file2
# Authors: Jim Klimov <EvgenyKlimov@eaton.com>

if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
#echo "INFO: Program '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..." >&2

[ -z "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"
# By default we do sorted comparisons; pass a " " space envvar to unset options
[ -z "$JSONSH_OPTIONS" ] && JSONSH_OPTIONS="-N=-n"
[ -z "$JSONSH_OPTIONS_VERBOSE" ] && JSONSH_OPTIONS_VERBOSE="-S=-n"

die() {
    [ -n "$CODE" -a "$CODE" -ge 0 ] 2>/dev/null || CODE=1
    echo "ERROR: $1"
    exit $CODE
}

self_test() {
    local jsonstr1='{"current":[{"id":3,"realpower.1":1,"voltage.2":1,"current.2":12,"current.1":31,"voltage.1":3}]}'
    local jsonstr2='{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.1":3}]}'
    local jsonstr3='{"current":[{"id":3,"realpower.1":1,"current.2":12,"current.1":31,"voltage.2":1,"voltage.2":3}]}'

    echo "=== This test should show no differences if JSON content sorting is enabled:"
    cmpjson_strings "$jsonstr1" "$jsonstr2" || die "json1 should equal to json2 (when sorted)"

    echo "=== This test should find some differences:"
    cmpjson_strings "$jsonstr1" "$jsonstr3" && die "json1 should NOT equal to json3"

    :
}

cmpjson_strings() {
    normstr1="`echo "$1" | eval $JSONSH $JSONSH_OPTIONS`"
    res1=$?
    normstr2="`echo "$2" | eval $JSONSH $JSONSH_OPTIONS`"
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
            trap "rm -f '$TMPF1' '$TMPF2'" 0 1 2 3 15 && \
            { echo "$1" | eval $JSONSH -l $JSONSH_OPTIONS_VERBOSE > "$TMPF1"; res1=$?
              echo "$2" | eval $JSONSH -l $JSONSH_OPTIONS_VERBOSE > "$TMPF2"; res2=$?
              [ "$res1" = 0 -a "$res2" = 0 ] && diff -bu "$TMPF1" "$TMPF2"; }
            rm -f "$TMPF1" "$TMPF1"
            trap '' 0 1 2 3 15
	fi
    fi
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
    # Open files
    eval exec "$FD1<'$file1'" || return
    eval exec "$FD2<'$file2'" || return
    RES=0
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

        [ "$eof1" = 0 -a "$eof2" = 0 ] && { \
            cmpjson_strings "$data1" "$data2" \
            || RES=$(($RES+1)); }
    done
    # Close files
    eval exec "$FD1>&-"
    eval exec "$FD2>&-"
    if [ "$eof1" = 0 -o "$eof2" = 0 ]; then
        echo "ERROR: read $count1 lines, and '$file1' EOF=$eof1 while '$file2' EOF=$eof2" >&2
        [ "$RES" = 0 ] && RES=126
    fi
    [ $RES != 0 ] && \
        echo "ERROR: files '$file1' and '$file2' do not contain equivalent JSON content" >&2
    return $RES
}

usage() {
        echo "Usage: $0 {file1} {file2}"
        echo "  The two files should contain the same amount of single-line JSON documents"
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

[ -z "$JSONSH" -o ! -x "$JSONSH" ] && die "JSON.sh is not executable (tried '$JSONSH')"
case "$1" in
    -t)
        self_test
        exit
	;;
    -s) cmpjson_strings "$2" "$3"; exit ;;
    -h|--help)
        usage
        exit 0
	;;
esac

[ $# != 2 ] && usage && die "Bad number of parameters ($#)!"
[ ! -r "$1" -o ! -r "$2" ] && usage && die "Not readable files '$1' and '$2' were provided!"

cmpjson_files "$1" "$2"
