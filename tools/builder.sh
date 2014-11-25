#!/bin/sh
#
#   Copyright (c) 2014 Eaton Corporation <www.eaton.com>
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
#
#   Description: Script to generate all required files from fresh
#   git checkout and automate the fastest possible build (parallel
#   for speed, then sequential for reliability).
#   NOTE: It expects to be run from the root of the project directory
#   (probably the checkout directory, unless you use strange set-ups)
#   i.e. as "./tools/builder.sh" or, for simplicity, by passing its
#   supported command-line parameters to the main "autogen.sh" script.

# Some of our CI-scripts can define the CHECKOUTDIR variable
if [ x"$CHECKOUTDIR" != x ]; then
    cd "$CHECKOUTDIR" || exit
else
    cd "`dirname $0`/.."
fi

if [ x"$AUTOGEN_DONE" != xyes -a -x "./autogen.sh" ]; then
    # Variable was set by our autogen.sh if it invokes this script,
    # otherwise run the autogen logic first
    "./autogen.sh" || exit
fi

if [ ! -s "./configure" -o ! -x "./configure" ]; then
    echo "builder.sh: error: configure does not exist or is not executable!" 1>&2
    exit 1
fi

# For sub-dir build - automatic naming according to OS/arch of the builder
[ x"$BLDARCH" = x ] && BLDARCH="`uname -s`-`uname -m`"
[ x"$DESTDIR" = x ] && DESTDIR="/var/tmp/bios-core-instroot-${BLDARCH}"

# Set up the parallel make with reasonable limits, using several ways to
# gather and calculate this information
[ x"$NCPUS" = x ] && { NCPUS="`/usr/bin/getconf _NPROCESSORS_ONLN`" || NCPUS="`/usr/bin/getconf NPROCESSORS_ONLN`" || NCPUS="`cat /proc/cpuinfo | grep -wc processor`" || NCPUS=1; }
[ x"$NCPUS" != x -a "$NCPUS" -ge 1 ] || NCPUS=1
[ x"$NPARMAKES" = x ] && { NPARMAKES="`echo "$NCPUS*2"|bc`" || NPARMAKES="$(($NCPUS*2))" || NPARMAKES=2; }
[ x"$NPARMAKES" != x -a "$NPARMAKES" -ge 1 ] || NPARMAKES=2

do_make() {
	if [ ! -s Makefile ]; then
		case "$*" in
		    *clean*)
				echo "INFO: Makefile absent, skipping 'make $@'"
#distclean?#			[ -d config ] && rm -rf config
				return 0 ;;
		    *)
				echo "ERROR: Makefile absent, skipping 'make $@'" >&2
				return 1 ;;
		esac
	fi
	echo "INFO: running 'make "$@"'"
	case "$*" in
	    *distclean*)
		### Hack to avoid running configure if it is newer
		### than Makefile - these are deleted soon anyway
		echo "INFO: Hack to avoid extra useless configure - touch some files"
		touch Makefile
		[ -f config.status ] && touch config.status
		;;
	esac

	make "$@"; RES=$?

	[ "$RES" != 0 ] && case "$*" in
	    *-k*distclean*)
		### Killing files enforces that the project is reconfigured
		### during later make attempts (this protects against some
		### broken contents of a Makefile). Depending on the result
		### of "rm -f" protects against unwritable directories.
		rm -rf Makefile config.status config/ && \
		echo "WARN: 'make $@' failed ($RES) but we ignore it for cleanups" >&2 && \
		RES=0
		;;
	esac

	return $RES
}

buildSamedir() {
	do_make -k distclean
	./configure && \
	{ do_make -k clean; \
	  if [ x"$NOPARMAKE" != xY ]; then 
	    echo "=== PARMAKE:"; make V=0 -j $NPARMAKES -k "$@"; fi; \
	  echo "=== SEQMAKE:"; make "$@"; }
}

buildSubdir() {
	do_make -k distclean
	( { rm -rf build-${BLDARCH}; \
	  mkdir build-${BLDARCH}; \
	  cd build-${BLDARCH}; } && \
	../configure && \
	{ if [ x"$NOPARMAKE" != xY ]; then
	    echo "=== PARMAKE:"; make V=0 -j $NPARMAKES -k "$@"; fi; \
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

_WARNLESS_UNUSED=0
suppressWarningsUnused() {
	[ "$_WARNLESS_UNUSED" != 0 ] && return
	CFLAGS="$CFLAGS -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable"
	CXXFLAGS="$CXXFLAGS -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable"
	export CFLAGS CXXFLAGS
	echo "INFO: Fixed up CFLAGS and CXXFLAGS to ignore warnings about unused code"
	_WARNLESS_UNUSED=1
}
### The flag can be set in environment rather than passed on command line
[ x"$WARNLESS_UNUSED" = xyes ] && suppressWarningsUnused

while [ $# -gt 0 ]; do
	case "$1" in
	    "--warnless-unused")
		suppressWarningsUnused
		shift
		;;
	    *)	break ;;
	esac
done

case "$1" in
    "")
	# Ensure that an exit at this point is "successful"
	true
	;;
    build-samedir|build|make|make-samedir)
	shift
	buildSamedir "$@"
	exit
	;;
    build-subdir|make-subdir)
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
    distclean)
	./configure && \
	do_make -k distclean
	;;
    distcheck)
	do_make -k distclean
	./configure && \
	do_make distcheck
	;;
    conf|configure)
	do_make -k distclean
	./configure
	;;
    *)	echo "Usage: $0 [--warnless-unused] [ { build-samedir | build-subdir | install-samedir | install-subdir } [maketargets...]]"
	echo "This script (re-)creates the configure script and optionally either just builds"
	echo "or builds and installs into a DESTDIR the requested project targets."
	echo "For output clarity you can avoid the parallel pre-build step with export NOPARMAKE=Y"
	echo "Some uses without further parameters:"
	echo "Usage: $0 distcheck	- execute the distclean, configure and make distcheck"
	echo "Usage: $0 configure	- execute the distclean and configure step and exit"
	echo "Usage: $0 distclean	- execute the distclean step and exit"
	exit 2
	;;
esac
