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
# Otherwise we define it ourselves to use below
if [ x"$CHECKOUTDIR" = x ]; then
    CHECKOUTDIR="`dirname $0`/.."
fi
cd "$CHECKOUTDIR" || exit
# Actually... (re)define the value to a complete FS path
CHECKOUTDIR="`pwd`" || exit
export CHECKOUTDIR
echo "INFO: Starting '`basename $0` $@' for workspace CHECKOUTDIR='$CHECKOUTDIR'..."

VERB_COUNT=0
verb_run() {
	VERB_COUNT="$(($VERB_COUNT+1))" 2>/dev/null || \
	VERB_COUNT="`echo $VERB_COUNT+1|bc`" 2>/dev/null || \
	VERB_COUNT=""

    (
	### Despite the round parentheses, $$ contains the parent bash PID
	TAG="$$"
	[ x"$BASHPID" != x ] && TAG="$BASHPID"
	[ x"$VERB_COUNT" != x ] && TAG="$VERB_COUNT:$TAG"

	echo "INFO-RUN[${TAG}]: `date`: running '"$@"' from directory '`pwd`'..."
	"$@"
	RES=$?
	[ "$RES" = 0 ] && \
	    echo "INFO-RUN[${TAG}]: `date`: completed '"$@"' from directory '`pwd`'" || \
	    echo "INFO-RUN[${TAG}]: `date`: failed($RES) '"$@"' from directory '`pwd`'"

	### This line intentionally left blank :)
	echo ""
	return $RES
    )
}

if [ x"$AUTOGEN_DONE" != xyes -a -x "./autogen.sh" ]; then
    # Variable was set by our autogen.sh if it invokes this script,
    # otherwise run the autogen logic first
    verb_run ./autogen.sh || exit
fi

if [ ! -s "./configure" -o ! -x "./configure" ]; then
    echo "builder.sh: error: configure does not exist or is not executable!" 1>&2
    exit 1
fi

# For sub-dir build - automatic naming according to OS/arch of the builder
[ x"$BLDARCH" = x ] && BLDARCH="`uname -s`-`uname -m`"
[ x"$DESTDIR" = x ] && DESTDIR="/var/tmp/bios-core-instroot-${BLDARCH}"
# Name of the sub-directory for the build, relative to workspace root
# ...or absolute (i.e. in /tmp/build-test) - this also works ;)
[ x"$BUILDSUBDIR" = x ] && BUILDSUBDIR="build-${BLDARCH}"

# Set up the parallel make with reasonable limits, using several ways to
# gather and calculate this information
[ x"$NCPUS" = x ] && { NCPUS="`/usr/bin/getconf _NPROCESSORS_ONLN`" || NCPUS="`/usr/bin/getconf NPROCESSORS_ONLN`" || NCPUS="`cat /proc/cpuinfo | grep -wc processor`" || NCPUS=1; }
[ x"$NCPUS" != x -a "$NCPUS" -ge 1 ] || NCPUS=1
[ x"$NPARMAKES" = x ] && { NPARMAKES="`echo "$NCPUS*2"|bc`" || NPARMAKES="$(($NCPUS*2))" || NPARMAKES=2; }
[ x"$NPARMAKES" != x -a "$NPARMAKES" -ge 1 ] || NPARMAKES=2

# Normalize the optional flags
case "$NOPARMAKE" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	NOPARMAKE=yes ;;
    *)	NOPARMAKE=no  ;;
esac

case "$WARNLESS_UNUSED" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	WARNLESS_UNUSED=yes ;;
    *)	WARNLESS_UNUSED=no  ;;
esac

do_make() {
	if [ ! -s Makefile ]; then
		case "$*" in
		    *clean*)
				echo "INFO: Makefile absent, skipping 'make $@' for a cleaning action"
#distclean?#			[ -d config ] && rm -rf config
				return 0 ;;
		    *)
				echo "ERROR: Makefile absent, can not fulfill 'make $@'" >&2
				return 1 ;;
		esac
	fi
	case "$*" in
	    *distclean*)
		### Hack to avoid running configure if it is newer
		### than Makefile - these are deleted soon anyway
		echo "INFO: Take steps to avoid extra useless configure before distclean - touch some files"
		touch Makefile
		[ -f config.status ] && touch config.status
		;;
	esac

	verb_run make "$@"; RES=$?

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

do_build() {
	if [ x"$NOPARMAKE" != xyes ]; then 
	    echo "=== PARMAKE:"
	    do_make V=0 -j $NPARMAKES -k "$@" || true
	else
	    echo "=== PARMAKE disabled by user request"
	fi

	echo "=== SEQMAKE:"
	do_make "$@"
}

buildSamedir() {
	do_make -k distclean
	verb_run ./configure && \
	{ do_make -k clean; do_build "$@"; }
}

buildSubdir() {
	do_make -k distclean
	( { echo "INFO: (Re-)Creating the relocated build directory in '${BUILDSUBDIR}'..."
	  rm -rf "${BUILDSUBDIR}"; \
	  mkdir "${BUILDSUBDIR}" && \
	  cd "${BUILDSUBDIR}"; } && \
	verb_run "$CHECKOUTDIR/configure" && \
	do_build "$@" )
}

installSamedir() {
	{ do_make "DESTDIR=${DESTDIR}" install; }
}

installSubdir() {
	( cd "${BUILDSUBDIR}" && \
	  do_make "DESTDIR=${DESTDIR}" install )
}

_WARNLESS_UNUSED=0
suppressWarningsUnused() {
	[ "$_WARNLESS_UNUSED" != 0 ] && return
	CFLAGS="$CFLAGS -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable"
	CXXFLAGS="$CXXFLAGS -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable"
	CPPFLAGS="$CPPFLAGS -Wno-unused-variable -Wno-unused-parameter -Wno-unused-but-set-variable"
	export CFLAGS CXXFLAGS CPPFLAGS
	echo "INFO: Fixed up CFLAGS, CXXFLAGS and CPPFLAGS to ignore warnings about unused code"
	_WARNLESS_UNUSED=1
}

while [ $# -gt 0 ]; do
	case "$1" in
	    "--warnless-unused")
		WARNLESS_UNUSED=yes
		shift
		;;
	    --noparmake|--disable-parallel-make)
		NOPARMAKE=yes
		shift
		;;
	    --parmake|--enable-parallel-make)
		NOPARMAKE=no
		shift
		;;
	    *)	break ;;
	esac
done

### The flags can be set in environment rather than passed on command line
[ x"$WARNLESS_UNUSED" = xyes ] && suppressWarningsUnused

case "$1" in
    "")
	# Ensure that an exit at this point is "successful"
	true
	;;
    make|make-samedir)
	shift
	do_build "$@"
	exit
	;;
    make-subdir)
	shift
	( cd "${BUILDSUBDIR}" && \
	  do_build "$@" )
	exit
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
    distclean)
	verb_run ./configure && \
	do_make -k distclean
	;;
    distcheck)
	do_make -k distclean
	verb_run ./configure && \
	do_make distcheck
	;;
    conf|configure)
	do_make -k distclean
	verb_run ./configure
	;;
    *)	echo "Usage: $0 [--warnless-unused] [--disable-parallel-make] \ "
	echo "    [ { build-samedir | build-subdir | install-samedir | install-subdir \ "
	echo "      | make-samedir  | make-subdir } [maketargets...] ]"
	echo "This script (re-)creates the configure script and optionally either just rebuilds,"
	echo "or rebuilds and installs into a DESTDIR, or makes the requested project targets."
	echo "Note that the 'make' actions do not involve clearing and reconfiguring the build area."
	echo "For output clarity you can avoid the parallel pre-build step with export NOPARMAKE=Y"
	echo "Some uses without further parameters:"
	echo "Usage: $0 distcheck	- execute the distclean, configure and make distcheck"
	echo "Usage: $0 configure	- execute the distclean and configure step and exit"
	echo "Usage: $0 distclean	- execute the distclean step and exit"
	exit 2
	;;
esac
