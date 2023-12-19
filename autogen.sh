#!/bin/sh
#
#   Copyright (c) 1991-2012 iMatix Corporation <www.imatix.com>
#   Copyright (c) 2014 - 2020 Eaton
#   Copyright other contributors as noted in the AUTHORS file.
#
#   This file is part of the Eaton 42ity project.
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
#! \file   autogen.sh
#  \brief  Script to generate all required files from fresh git checkout
#  \note   It expects to be run in the root of the project directory
#          (probably the checkout directory, unless you use strange set-ups).

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

command -v libtool >/dev/null 2>&1 || command -v libtoolize >/dev/null 2>&1
if  [ $? -ne 0 ]; then
    echo "autogen.sh: error: could not find libtool nor libtoolize." 1>&2
    echo "ERROR: libtool(ize) is required to run autogen.sh." 1>&2
    exit 1
fi

command -v autoreconf >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "autogen.sh: error: could not find autoreconf.  autoconf and automake are required to run autogen.sh." 1>&2
    exit 1
fi

command -v pkg-config >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "autogen.sh: error: could not find pkg-config.  pkg-config is required to run autogen.sh." 1>&2
    exit 1
fi

if [ ! -d ./config ]; then
    mkdir -p ./config
    if [ $? -ne 0 ]; then
        echo "autogen.sh: error: could not create directory: ./config." 1>&2
        exit 1
    fi
fi

# This flag changes to yes if some obsolete/missing files are found
# and unless FORCE_AUTORECONF=no, will trigger an autoreconf/automake
# For FORCE_AUTORECONF="no" or "auto", we trace all such dependencies
# to be at least informed of whether changes should be done, anyhow.
SHOULD_AUTORECONF=no
[ x"$FORCE_AUTORECONF" != xyes -a x"$FORCE_AUTORECONF" != xno ] && \
    FORCE_AUTORECONF=auto

# Likewise, detect if prerequisite packages needed for the build should
# automagically be installed (creates a core-build-deps_0.1_all.deb in
# workspace directory); root privs or sudo permissions may be required
SHOULD_MKBUILDDEPS=no
[ x"$FORCE_MKBUILDDEPS" != xyes -a x"$FORCE_MKBUILDDEPS" != xno ] && \
    FORCE_MKBUILDDEPS=auto

if [ x"$FORCE_AUTORECONF" != xyes ]; then
    if [ ! -s "./configure" ]; then
        echo "autogen.sh: info: configure does not exist."
        SHOULD_AUTORECONF=yes
    else
        _OUT="`find . -maxdepth 1 -type f -name configure -newer configure.ac`"
        if [ $? != 0 -o x"$_OUT" != x"./configure" ]; then
            echo "autogen.sh: info: configure is older than configure.ac."
            SHOULD_AUTORECONF=yes
        fi

        if [ x"$SHOULD_AUTORECONF" = xno -o x"$FORCE_AUTORECONF" = xno ]; then
            _OUT="`find ./m4/ -type f -name '*.m4' -newer configure`"
            if [ $? != 0 -o x"$_OUT" != x ]; then
                echo "autogen.sh: info: configure is older than some ./m4/*.m4 files:"
                echo "$_OUT"
                SHOULD_AUTORECONF=yes
            fi
        fi
    fi

    [ x"$SHOULD_AUTORECONF" = xno -o x"$FORCE_AUTORECONF" = xno ] && \
    for M_am in `find . -name Makefile.am`; do
        DIR="`dirname ${M_am}`"
        if [ ! -s "$DIR/Makefile.in" ]; then
            echo "autogen.sh: info: Missing $DIR/Makefile.in"
            SHOULD_AUTORECONF=yes
        else
            _OUT="`cd "$DIR" && find . -maxdepth 1 -type f -name Makefile.in -newer Makefile.am`"
            if [ $? != 0 -o x"$_OUT" != x"./Makefile.in" ]; then
                echo "autogen.sh: info: $DIR/Makefile.in is older than $DIR/Makefile.am."
                SHOULD_AUTORECONF=yes
            fi
        fi
    done
fi

case x"$FORCE_AUTORECONF" in
    xauto)
        if [ x"$SHOULD_AUTORECONF" = xyes ]; then
            FORCE_AUTORECONF=yes
        else
            echo "autogen.sh: info: no prerequisite changes detected for the configure script or Makefiles."
        fi
        ;;
    xno)
        if [ x"$SHOULD_AUTORECONF" = xyes ]; then
            echo "autogen.sh: info: not rebuilding the configure script due to explicit request, but prerequisite changes were detected for the configure script or Makefiles." >&2
        fi # else = don't want and don't have to rebuild configure, noop
        ;;
esac

if [ x"$FORCE_AUTORECONF" = xyes ]; then
    echo "autogen.sh: info: rebuilding the configure script and Makefiles."
    autoreconf --install --force --verbose -I config
    RES=$?
    if [ $RES -ne 0 ]; then
        echo "autogen.sh: error: autoreconf exited with status $RES" 1>&2
        exit 1
    fi
fi

chmod +x "./configure"

if [ ! -s "./configure" -o ! -x "./configure" ]; then
    echo "autogen.sh: error: configure does not exist or is not executable!" 1>&2
    exit 1
fi

MKBD_DSC="`dirname $0`/obs/core.dsc"
MKBD_CTL="`dirname $0`/obs/debian.control"
MKBD_DEB_PATTERN='*-build-deps_*_all.deb'
MKBD_DEB="`ls -1 $MKBD_DEB_PATTERN 2>/dev/null | egrep 'bios|core' | head -1`" || \
    MKBD_DEB=""
[ -z "$MKBD_DEB" ] && MKBD_DEB="core-build-deps_0.1_all.deb"

if which mk-build-deps >/dev/null && which apt-get > /dev/null && [ -s "$MKBD_DSC" ] ; then
    # mk-build-deps and debian packaging are at all supported
    if [ x"$FORCE_MKBUILDDEPS" != xyes ]; then
        if [ ! -s "$MKBD_DEB" ]; then
            echo "autogen.sh: info: $MKBD_DEB does not exist."
            SHOULD_MKBUILDDEPS=yes
        else
            _OUT="`find . -maxdepth 1 -type f -name "$MKBD_DEB_PATTERN" \! -newer "$MKBD_DSC"`"
            if [ $? != 0 -o x"$_OUT" != x"" ]; then
                echo "autogen.sh: info: $MKBD_DEB is older than $MKBD_DSC."
                SHOULD_MKBUILDDEPS=yes
            else
                _OUT="`find . -maxdepth 1 -type f -name "$MKBD_DEB_PATTERN" \! -newer "$MKBD_CTL"`"
                if [ $? != 0 -o x"$_OUT" != x"" ]; then
                    echo "autogen.sh: info: $MKBD_DEB is older than $MKBD_CTL."
                    SHOULD_MKBUILDDEPS=yes
                fi
            fi
        fi
    fi

    case x"$FORCE_MKBUILDDEPS" in
    xauto)
        if [ x"$SHOULD_MKBUILDDEPS" = xyes ]; then
            FORCE_MKBUILDDEPS=yesauto
        else
            echo "autogen.sh: info: no prerequisite package requirement changes detected."
        fi
        ;;
    xno)
        if [ x"$SHOULD_MKBUILDDEPS" = xyes ]; then
            echo "autogen.sh: info: not verifying and perhaps installing prerequisite packages, even though the system may be obsolete." >&2
        fi # else = don't want and don't have to verify pkgs, noop
        ;;
    esac

    if [ x"$FORCE_MKBUILDDEPS" = xyes -o x"$FORCE_MKBUILDDEPS" = xyesauto ]; then
        echo "autogen.sh: info: Making sure all needed packages are installed (note: may try to elevate privileges)."
        { echo "apt-get: Trying direct invokation..."
          apt-get update -q; } || \
        { echo "apt-get: Retrying sudo..."
          sudo apt-get update -q || { echo "Wipe metadata and retry"; sudo rm -rf /var/lib/apt/lists/* && sudo apt-get update -q; } ; } || \
        { echo "apt-get: Retrying su..."
          su - -c "apt-get update -q || { echo 'Wipe metadata and retry'; rm -rf /var/lib/apt/lists/*; apt-get update -q; } "; }

        echo "mk-build-deps: generate package file" && \
        mk-build-deps "$MKBD_DSC" && [ -s "$MKBD_DEB" ] && \
        { echo "mk-build-deps install: Trying direct invokation..."
          export DEBIAN_FRONTEND=noninteractive
          dpkg --force-all -i "$MKBD_DEB" && \
          apt-get --yes --force-yes -f -q \
                -o Dpkg::Options::="--force-confdef" \
                -o Dpkg::Options::="--force-confold" \
                install && \
          dpkg --configure -a ; } || \
        { echo "mk-build-deps install: Retrying sudo..."
          sudo sh -c "export DEBIAN_FRONTEND=noninteractive ; dpkg --force-all -i '$MKBD_DEB' && apt-get --yes --force-yes -f -q -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' install && dpkg --configure -a"; } || \
        { echo "mk-build-deps install: Retrying su..."
          su - -c "export DEBIAN_FRONTEND=noninteractive ; dpkg --force-all -i '$MKBD_DEB' && apt-get --yes --force-yes -f -q -o Dpkg::Options::='--force-confdef' -o Dpkg::Options::='--force-confold' install && dpkg --configure -a"; }
        RES=$?
        if [ $RES -ne 0 ]; then
            echo "autogen.sh: error: mk-build-deps exited with status $RES" 1>&2
            [ x"$FORCE_MKBUILDDEPS" = xyes ] && \
                exit 1 || \
                echo "...continuing with autogen, but it may fail later due to this." >&2
        fi
    fi
fi # clause-if we try mk-build-deps at all?

if [ $# = 0 ]; then
    # Ensure that an exit at this point is "successful"
    exit 0
fi

# Use up the hook into the build-automation routine
if [ -x "`dirname $0`/tools/builder.sh" ]; then
    AUTOGEN_DONE=yes
    export AUTOGEN_DONE
    echo "autogen.sh: info: calling the builder script to automate the rest of compilation."
    exec "`dirname $0`/tools/builder.sh" "$@"
fi

