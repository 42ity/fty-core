#!/bin/sh
#
#   Copyright (c) 1991-2012 iMatix Corporation <www.imatix.com>
#   Copyright (c) 2014 Eaton Corporation <www.eaton.com>
#   Copyright other contributors as noted in the AUTHORS file.
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
#   Description: Script to generate all required files from fresh git
#   checkout.
#   NOTE: It expects to be run in the root of the project directory
#   (probably the checkout directory, unless you use strange set-ups).

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
export LANG LANGUAGE LC_ALL

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

