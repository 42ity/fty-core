#!/bin/bash
#
#   Copyright (c) 2015 - 2020 Eaton
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
#! \file    curlbbwget.sh
#  \brief   Emulator of certain curl functionality by using BusyBox limited WGET
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details The intended use is in 42ity REST API testing/usage, so
#           implements just as much BASH required due to use of arrays

[ -z "$MULTIPART_BOUNDARY" ] && \
    MULTIPART_BOUNDARY='--------------------------b10c0fda7a424242'
[ -z "$DRY_RUN" ] && DRY_RUN=no
[ -z "$CURLSTDERR" ] && CURLSTDERR='&2'

# Note that array elements are passed as singular string tokens, so there
# must(!) be no quoting inside, e.g. "--flag=arg with space" needs no quotes!
declare -a WGETARGS=()
declare -a WGETARGS_URL=()
# Some options are not supported by BusyBox, or not by all builds
# TODO: Actually use values collected here based on runtime detected support?
declare -a WGETARGS_GNU=()

echo_E() { /bin/echo -E "$@"; }
echo_concat() { /bin/echo -e "$@""\c"; }

parsecmd() {
    # Prints out bits of command line that will be passed to wget below
    # Assumes BusyBox wget, can likely work wit GNU wget as well ;)
    WGETARGS+=("-q" "-O" "-")

    # Prepare the CRLF-separated string
    FORM_TAIL="`echo "
${MULTIPART_BOUNDARY}--" | sed 's,$,\r,'`"
    FORM_POST=""
    FORM_HEAD=""
    FORM_DATA=""

    while [ $# -gt 0 ]; do
        #echo "... '$1'" >&2
        case "$1" in
            -v) ;; # silently ignore
            --stderr) CURLSTDERR="$2"; shift ;;
            --insecure) WGETARGS+=("--no-check-certificate") ;;
            --progress-bar) WGETARGS_GNU+=("--show-progress") ;;
            --header|-H) case "$2" in
                    "Expect:") ;;
                    *) WGETARGS+=("--header=$2") ;;
#                    *) WGETARGS+=("--header" "`echo "$2" | sed 's, ,\\ ,g'`") ;;
#                    *) WGETARGS+=(" --header=`echo "$2" | sed 's,\([^:]\) ,\1\"\\ \",g' | sed 's,^\([^:]*\:\) ,\1,'`") ;;
                esac
                shift ;;
            --data|-d) 
                #WGETARGS+=("--post-data=\"`echo "$2" | sed 's, ,\\\\ ,g'`\"")
                WGETARGS+=("--post-data=$2")
                shift ;;
            --form|-F) # TODO!
                FORM_CONTENT_TYPE="application/octet-stream"
                FORM_FILE_NAME=""
                FORM_PART_NAME=""
                case "$2" in
                    *=*) FORM_PART_NAME="`echo_E "$2" | sed -e 's,^\([^=;]*\)=.*$,\1,'`" || FORM_PART_NAME="";;
                esac
                [ -z "$FORM_PART_NAME" ] && \
                    echo "ERROR: Form argument must be 'name=[<@]content[;mod=arg;...]'" >&2 && \
                    exit 1

                case "$2" in
                    *';'type=*) FORM_CONTENT_TYPE="`echo_E "$2" | sed -e 's,^.*;type=,,' -e 's,;.*$,,'`" ;;
                esac
                case "$2" in
                    *';'filename=*) # Explicit filename requested to override
                        FORM_FILE_NAME="`echo_E "$2" | sed -e 's,^.*;filename=,,' -e 's,;.*$,,'`"
                        ;;
                    *='@'-*) # '-' for stdin
                        FORM_FILE_NAME="_stdin_"
                        ;;
                    *='@'*) # do only the input filename (@)
                            # don't volunteer for in-place (<)
                            # don't be too smart and too difficult ;)
                        FORM_FILE_NAME="`echo_E "$2" | sed -e 's,^[^=@]*=@,,' -e 's,;.*$,,' -e 's,^\"\(.*\)\"$,\1,' -e 's,^.*/\([^/]*\)$,\1,'`" || \
                            FORM_FILE_NAME=""
                        [ -z "$FORM_FILE_NAME" ] && FORM_FILE_NAME="_unknown_"
                        ;;
                    *) ;; # Not a file upload
                esac
                FORM_HEAD="$MULTIPART_BOUNDARY
Content-Disposition: form-data; name=\"$FORM_PART_NAME\""
                [ -n "$FORM_FILE_NAME" ] && FORM_HEAD="$FORM_HEAD; filename=\"$FORM_FILE_NAME\""
                [ -n "$FORM_CONTENT_TYPE" ] && FORM_HEAD="$FORM_HEAD
Content-Type: $FORM_CONTENT_TYPE"
                FORM_HEAD="`echo_E "$FORM_HEAD
" | sed 's,$,\r,'`"
                case "$2" in
                    *='<'*|*='@'*) # form data is file content, '-' for stdin
                        # '<' = prepared data ; '@' = named file upload
                        FILE_NAME="`echo_E "$2" | sed -e 's,^[^=@<]*=[@<],,' -e 's,;.*$,,' -e 's,^\"\(.*\)\"$,\1,'`" || \
                            FILE_NAME=""
                        case "$FILE_NAME" in
                            -) FORM_DATA="`cat`" || exit $? ;;
                            *) FORM_DATA="`cat "$FILE_NAME"`" || exit $? ;;
                        esac
                        ;;
                    *=*) # form data is the follow-up string
                        FORM_DATA="`echo_E "$2" | sed -e 's,^[^=;]*=,,' -e 's,;.*$,,'`"
                        ;;
                esac
                FORM_POST="${FORM_POST}${FORM_HEAD}
${FORM_DATA}
${FORM_TAIL}
"
                shift ;;
            -X|--request) # NOTE: This is not in standard BusyBox wget!
                # TODO: Patch our builds of wget client, or emulate this
                # for REST API via headers and GET/POST server processing
                WGETARGS+=("--method=$2")
                shift ;;
            --dry-run|--dryrun) DRY_RUN=yes ;;
            *) # Fall through, assume URL or some native command
                #echo "GOT URL: '$1'" >&2
                WGETARGS_URL+=("$1")
                ;;
        esac
        shift
    done

    [ -n "$FORM_POST" ] && \
        WGETARGS+=("--post-data=$FORM_POST") && \
        WGETARGS+=("--header=Content-Type: multipart/form-data; boundary=$MULTIPART_BOUNDARY")

    return 0
}

debug_print() {
    echo_concat "DEBUG: $WGET "
    for I in "${WGETARGS[@]}" ; do
        echo_concat "'$I' "
    done
    echo_concat "  ... URLs: ...  "
    for I in "${WGETARGS_URL[@]}" ; do
        echo_concat "'$I' "
    done
    echo ""; echo ""
}

run_wget() {
    $WGET "${WGETARGS[@]}" "${WGETARGS_URL[@]}"
}

WGET=""
(which wget >/dev/null 2>&1) && WGET=wget || \
(which busybox >/dev/null 2>&1) && WGET="busybox wget"

[ -z "$WGET" ] && { echo "FATAL: a wget program is required for $0!">&2; exit 127;}

parsecmd "$@" || exit $?

if [ "$DRY_RUN" = yes ]; then
        echo "DEBUG: dry-running, should not call $WGET">&2
        debug_print >&2
        exit 0
else
        [ "$CI_DEBUG" -gt 3 ] 2>/dev/null && debug_print >&2
        # Direct stderr to &2 or a file
        eval run_wget "2>$CURLSTDERR"
        exit $?
fi
