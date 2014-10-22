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

#   Script to generate all required files from fresh git checkout.

command -v libtool >/dev/null 2>&1
if  [ $? -ne 0 ]; then
    echo "autogen.sh: error: could not find libtool.  libtool is required to run autogen.sh." 1>&2
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
    if [ -s ./configure.ac ]; then
	echo "autogen.sh: info: touching configure.ac to ensure rebuilding of configure script."
	touch ./configure.ac
    fi
fi

_OUT="`find . -maxdepth 1 -type f -name configure -newer configure.ac`"
if [ $? != 0 -o x"$_OUT" != x"./configure" ]; then
    echo "autogen.sh: info: configure does not exist or is older than configure.ac - rebuilding."
    autoreconf --install --force --verbose -I config
    if [ $? -ne 0 ]; then
	echo "autogen.sh: error: autoreconf exited with status $?" 1>&2
	exit 1
    fi
fi

if [ ! -s "./configure" -o ! -x "./configure" ]; then
    echo "autogen.sh: error: configure does not exist or is not executable!" 1>&2
    exit 1
fi

[ x"$BLDARCH" = x ] && BLDARCH="`uname -s`-`uname -m`"
[ x"$DESTDIR" = x ] && DESTDIR="/var/tmp/bios-core-instroot-${BLDARCH}"

buildSamedir() {
	make -k distclean
	./configure && \
	{ make -k clean; \
	  echo "=== PARMAKE:"; make V=0 -j 4 -k "$@"; \
	  echo "=== SEQMAKE:"; make "$@"; }
}

buildSubdir() {
	make -k distclean
	( { rm -rf build-${BLDARCH}; \
	  mkdir build-${BLDARCH}; \
	  cd build-${BLDARCH}; } && \
	../configure && \
	{ echo "=== PARMAKE:"; make V=0 -j 4 -k "$@"; \
	  echo "=== SEQMAKE:"; make "$@"; } && \
	{ make DESTDIR=${DESTDIR} install; } )
}

installSamedir() {
	{ make DESTDIR=${DESTDIR} install; }
}

installSubdir() {
	( cd build-${BLDARCH} && \
	  make DESTDIR=${DESTDIR} install )
}

case "$1" in
    "")
	# Ensure that an exit at this point is "successful"
	true
	;;
    build-samedir|build)
	shift
	buildSamedir "$@"
	exit
	;;
    build-subdir)
	shift
	buildSubdir "$@"
	exit
	;;
    install-samedir|install)
	shift
	buildSamedir "$@" && \
	installSamedir
	exit
	;;
    install-subdir)
	shift
	buildSubdir "$@" && \
	installSubdir
	exit
	;;
    distcheck)
	make -k distclean
	./configure && \
	make distcheck
	;;
    *)	echo "Usage: $0 [ { build-samedir | build-subdir | install-samedir | install-subdir } maketargets...]"
	echo "This scrpit (re-)creates the configure script and optionally either just builds"
	echo "or builds and installs into a DESTDIR the requested project targets."
	echo "Usage: $0 distcheck	- execute the configure and make distcheck"
	exit 2
	;;
esac
