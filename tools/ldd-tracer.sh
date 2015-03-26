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

### This variable will hold the list of unresolved symbols
### after an execution of this function
symbols_missing=""
trace_objfile() {
    target="$1"
    [ -z "$target" -o ! -s "$target" ] && \
        echo "ERROR: no valid target supplied!" >&2 && return 1

    shift
    libs_add="$@"

    _SYMBOLS="$(nm -D "$target" | grep -w "U" | while read _U _S; do echo "$_S"; done)"
    [ $? != 0 -o -z "$_SYMBOLS" ] && \
        echo "INFO: no external symbols detected in target '$target'" && \
        return 0

    # Recurse over shared libraries until we've found them all
    # (via dynamic linker explicitly declared dependencies).
    # TODO: Detect "file not found" link errors, etc.?
    _LPREV=""
    _LIBS="$target $libs_add"
    while [ x"$_LPREV" != x"$_LIBS" ]; do
        _LPREV="$_LIBS"
        _LIBS="$(for L in $_LPREV ; do echo "$L"; ldd "$L"; done | sed -e 's,^[ \t]*\([^ ].*\)$,\1,' -e 's,^.* => \(.*\)$,\1,' -e 's, (0x.*)$,,' | egrep -v '^$|^[^/]' | sort | uniq)"
    done
    unset _LPREV

    # See https://sourceware.org/binutils/docs/binutils/nm.html
    # for listing of symbol type codes in GNU nm
    _LIBSYMS="$(for library in $_LIBS ; do nm -D "$library" | egrep '[ \t][TtiAN][\t ]' | grep -vw U | while read _O _U lib_symbol; do echo "$lib_symbol $library"; done; done )"

    NUMMISS=0
    symbols_missing=""
    for symbol in $_SYMBOLS ; do
        echo "$_LIBSYMS" | egrep '^'"$symbol"'[ @]' | ( found=no; RES=0
            while read lib_symbol library; do
                echo -e "${_GRN}Found${_OFF} symbol: ${_MAG}$symbol${_OFF} at '${_BLU}$library${_OFF}' (as ${_BLU}$lib_symbol${_OFF})"
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
    echo "Usage: $0 [-l 'libX.so libY.so...'] [--color|--no-color] [targetfile]"
    echo "    -l        Singular or listed (as one token) additional libs to inspect"
    echo "    --color|--no-color        Colorize the output?"
    echo "    target    The dynamically-linked program or library to inspect" \
         "(default: $target_default)"
}

while [ $# -gt 0 ]; do
    case "$1" in
        -l) libs_add="$libs_add $2"; shift ;;
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
