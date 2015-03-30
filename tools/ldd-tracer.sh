#!/bin/bash
#
#   Copyright (c) 2015 Eaton Corporation <www.eaton.com>
#
#   This file is part of the Eaton $BIOS project.
#
#   This is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This software is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   Author(s): Jim Klimov <EvgenyKlimov@eaton.com>
#   Inspired by https://stackoverflow.com/questions/5108079/how-do-i-find-out-which-functions-of-a-shared-object-are-used-by-a-program-or-an/5108205#5108205?newreg=952b5d1d5de74317971e1fea530f27cf
#
#   Description: Script to trace dynamically-linked objects (bin, lib)
#   to find the libraries which define a target object's symbols and
#   if there are some undefined symbols as well.
#   NOTE: Uses bash syntax in the code (math, echo -e) and GNU nm.

### Input (see also command-line parameters below):
target_default="bios_web.so"
[ -z "$target" ] && target="$target_default"
### This allows to handle some libraries loaded during runtime (dlopen)
### rather than only the chain of pre-linked ones.
[ -z "$libs_add" ] && libs_add=""
### Use escape sequences for coloring?
[ -z "$do_color" ] && do_color=yes

LANG=C
LC_ALL=C
export LANG LC_ALL

set -o pipefail 2>/dev/null

### This variable will hold the list of unresolved symbols
### after an execution of this function
symbols_missing=""
trace_objfile() {
    target="$1"
    [ -z "$target" -o ! -s "$target" ] && \
        echo "ERROR: no valid target supplied!" >&2 && return 1

    _FT="`file "$target" 2>/dev/null`" || _FT=""
    case "${_FT}" in
        *ASCII*|*data*|*script*|*libtool*)
            echo "ERROR: target '$target' contents seem like a non-executable file:" >&2
            echo "    ${_FT}" >&2
            return 1
            ;;
    esac

    shift
    libs_add="$@"

    _SYMBOLS="$(nm -D "$target" | grep -w "U" | while read _U _S; do echo "$_S"; done | sort | uniq)"
    [ $? != 0 -o -z "$_SYMBOLS" ] && \
        echo "INFO: no external symbols detected in target '$target'" && \
        return 0

    # Recurse over shared libraries until we've found them all
    # (via dynamic linker explicitly declared dependencies).
    _LPREV=""
    _LIBS="$target $libs_add"
    _LIBSN=""
    while [ x"$_LPREV" != x"$_LIBS" ]; do
        _LPREV="$_LIBS"
        _LIBS="$(for L in $_LPREV ; do echo "$L"; ldd "$L"; done | sed -e 's,^[ \t]*\([^ ].*\)$,\1,' -e 's,^.* => \(.*\)$,\1,' -e 's, (0x.*)$,,' | egrep -v '^$|^[^/]|^'"$target"'$' | sort | uniq)"
        _LIBSN="$( (echo "$_LIBSN" ; for L in $_LPREV ; do ldd "$L"; done | grep 'not found' | awk '{print $1}' ) | sort|uniq|egrep -v '^$' )"
    done
    unset _LPREV

    if [ -n "$_LIBSN" ]; then
        echo "WARN: the following links to external libraries were not resolved in target '$target':"
        for L in $_LIBSN ; do
            echo -e "    ${_RED}$L${_OFF} : not found : shared library name not resolved into a path" >&2
        done
        echo ""
    fi

    if [ -z "$_LIBS" ]; then
        echo "INFO: no links to external libraries were found in target '$target'"
        return 0
    fi

    echo "Libraries to check:"
    for L in $_LIBS ; do
        [ -s "$L" ] && \
            echo -e "    ${_BLU}$L${_OFF}" || \
            echo -e "    ${_RED}$L${_OFF} : not found : shared library name is not a non-empty file" >&2
    done
    echo ""

    # See https://sourceware.org/binutils/docs/binutils/nm.html
    # for listing of symbol type codes in GNU nm
    # NOTE: depending on library contents, there may be duplicate
    # hits e.g. for "implementation-dependent" functions. While these
    # can be filetered away with "sort|uniq" in between two done's,
    # I chose to leave this as is for clarity during debugging.
    # TODO: attention to error codes?
    _LIBSYMS="$(for library in $_LIBS ; do nm -D "$library" | egrep '[ \t][TtiANWwVvBbDdGgu][\t ]' | grep -vw U | while read _O sym_type lib_symbol; do echo "$lib_symbol $sym_type $library"; done; done )"

    NUMMISS=0
    symbols_missing=""
    for symbol in $_SYMBOLS ; do
        echo "$_LIBSYMS" | egrep '^'"$symbol"'[ @]' | ( found=no; RES=0
            while read lib_symbol sym_type library; do
                TAG=""
                case "$sym_type" in
                    [WwVv]) TAG="${_MAG}WEAK${_OFF} " ;;
                    [BbDdGg]) TAG="${_MAG}DATA${_OFF} " ;;
                esac
                echo -e "${_GRN}Found${_OFF} symbol: ${_MAG}$symbol${_OFF} at '${_BLU}$library${_OFF}' (as ${TAG}${_BLU}$lib_symbol${_OFF} type '$sym_type')"
                found=yes
            done
            [ "$found" = no ] && RES=1 && \
                echo -e "Symbol ${_RED}NOT FOUND${_OFF} anywhere: ${_MAG}$symbol${_OFF}" >&2
            exit $RES )

        if [ $? != 0 ]; then
            [ $NUMMISS = 0 ] && symbols_missing="$symbol" || \
                symbols_missing="$symbols_missing
$symbol"
                NUMMISS=$(($NUMMISS+1))
        fi
    done
    return $NUMMISS
}

trace_objfile_wrap() {
    ### Pretty-print summary of the trace results as well

    trace_objfile "$@"
    NUM_BAD=$?
    if [ "$NUM_BAD" = 0 ]; then
        echo -e "${_GRN}SUCCESS${_OFF}"
    else
        [ -z "$symbols_missing" ] && \
            echo -e "${_RED}FAILED${_OFF}: usage error" || \
        echo -e "
${_RED}FAILED${_OFF}: $NUM_BAD non-local symbols not traced to any dynamic library:
$symbols_missing"
    fi

    return $NUM_BAD
}

usage() {
    echo "Usage: $0 [-l 'libX.so /a/libY.so...'] [-lp '.libs:/tmp/bld/.libs'] [--color|--no-color] [targetfile]"
    echo "    -l        Singular or space-separated list (as one token) additional"
    echo "              dynamic library and/or executable filenames to inspect"
    echo "    -lp       Prepend the path (single or colon-separated) to LD_LIBRARY_PATH"
    echo "    --color|--no-color        Colorize the output?"
    echo "    target    The dynamically-linked program or library to inspect" \
         "(default: $target_default)"
}

while [ $# -gt 0 ]; do
    case "$1" in
        -l) libs_add="$libs_add $2"; shift ;;
        -lp) LD_LIBRARY_PATH="$2:$LD_LIBRARY_PATH"
            export LD_LIBRARY_PATH
            shift ;;
        --color|--do-color) do_color=yes ;;
        --no-color) do_color=no ;;
        -h|--help) usage; exit 0;;
        *)  if [ $# = 1 ]; then
                target="$1"
            else
                echo "Unknown param: $1"
            fi ;;
    esac
    shift
done

### The escape patterns
if [ "$do_color" = yes ]; then
    _OFF='\033[0m'
    _RED='\e[1;31m'
    _GRN='\e[1;32m'
    _BLU='\e[1;34m'
    _MAG='\e[1;36m'
else
    _OFF=''
    _RED=''
    _GRN=''
    _BLU=''
    _MAG=''
fi

trace_objfile_wrap "$target" $libs_add
exit $?
