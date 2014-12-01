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

# This is a generally used variable in the build systems to
# override into usage of a specific make program filename/path
# Also a custom variable MAKE_OPTS can be used to pass flags to `make`
# Finally, for options specific to only one stage, the caller can set
# MAKE_OPTS_PAR or MAKE_OPTS_SEQ respectively (no defaults)
[ -z "$MAKE" ] && MAKE="make"
#default# MAKE_OPTS=""
#default# MAKE_OPTS_PAR=""
#default# MAKE_OPTS_SEQ=""
case "$MAKE" in
    *\ *) # Split into program and options
	_MAKE_OPTS="`echo "$MAKE" | { read _C _A; echo "$_A"; }`"
	MAKE="`echo "$MAKE" | { read _C _A; echo "$_C"; }`"
	if [ -n "$_MAKE_OPTS" ]; then
	    [ -n "$MAKE_OPTS" ] && \
		MAKE_OPTS="$MAKE_OPTS $_MAKE_OPTS" || \
		MAKE_OPTS="$_MAKE_OPTS"
	fi
	unset _MAKE_OPTS
	;;
esac

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

case "$SHOW_BUILDER_FLAGS" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	SHOW_BUILDER_FLAGS=yes ;;
    *)	SHOW_BUILDER_FLAGS=no  ;;
esac

do_make() {
	if [ ! -s Makefile ]; then
		case "$*" in
		    *clean*)
				echo "INFO: Makefile absent, skipping '$MAKE $@' for a cleaning action"
#distclean?#			[ -d config ] && rm -rf config
				return 0 ;;
		    *)
				echo "ERROR: Makefile absent, can not fulfill '$MAKE $@'" >&2
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

	verb_run $MAKE "$@"; RES=$?

	[ "$RES" != 0 ] && case "$*" in
	    *-k*distclean*)
		### Killing files enforces that the project is reconfigured
		### during later make attempts (this protects against some
		### broken contents of a Makefile). Depending on the result
		### of "rm -f" protects against unwritable directories.
		rm -rf Makefile config.status config/ && \
		echo "WARN: '$MAKE $@' failed ($RES) but we ignore it for cleanups" >&2 && \
		RES=0
		;;
	esac

	return $RES
}

do_build() {
	if [ x"$NOPARMAKE" != xyes ]; then 
	    echo "=== PARMAKE (fast first pass which is allowed to fail): $MAKE_OPTS_PAR $MAKE_OPTS $@"
	    case " $MAKE_OPTS_PAR $MAKE_OPTS $*" in
		*\ V=*|*\ --trace*)
		    do_make $MAKE_OPTS_PAR $MAKE_OPTS -j $NPARMAKES -k "$@" || true ;;
		*)
		    do_make V=0 $MAKE_OPTS_PAR -j $NPARMAKES -k "$@" || true ;;
		esac
	else
	    echo "=== PARMAKE disabled by user request"
	fi

	# User can request 'builder.sh install-subdir V=0' or somesuch
	# to suppress the build tracing, or '... --trace' to increase it
	# ...or the MAKE variable can be overridden to the same effect
	echo "=== SEQMAKE: $MAKE_OPTS_SEQ $MAKE_OPTS $@"
	do_make $MAKE_OPTS_SEQ $MAKE_OPTS "$@"
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

usage() {
	echo "Usage: $0 [--warnless-unused] [--disable-parallel-make] [--show-builder-flags] \ "
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
}

showGitFlags() {
    # Get the Git repository metadata, if available
    # Code cloned from our configure.ac
    HAVE_PACKAGE_GIT=0
    PACKAGE_GIT_ORIGIN=""
    PACKAGE_GIT_BRANCH=""
    PACKAGE_GIT_TSTAMP=""
    PACKAGE_GIT_HASH_S=""
    PACKAGE_GIT_HASH_L=""

    [ -z "$GIT" ] && GIT="`which git 2>/dev/null | head -1`"
    _srcdir_abs="$CHECKOUTDIR"

    if test ! -z "$GIT" -a -x "$GIT" -a -d "$_srcdir_abs/.git" ; then
        PACKAGE_GIT_ORIGIN="`cd "$_srcdir_abs" && $GIT config --get remote.origin.url`"	&& HAVE_PACKAGE_GIT=1
        PACKAGE_GIT_BRANCH="`cd "$_srcdir_abs" && $GIT rev-parse --abbrev-ref HEAD`"	&& HAVE_PACKAGE_GIT=1
        PACKAGE_GIT_TSTAMP="`cd "$_srcdir_abs" && $GIT log -n 1 --format='%ct'`"	&& HAVE_PACKAGE_GIT=1
        PACKAGE_GIT_HASH_S="`cd "$_srcdir_abs" && $GIT log -n 1 --format='%h'`"		&& HAVE_PACKAGE_GIT=1
        PACKAGE_GIT_HASH_L="`cd "$_srcdir_abs" && $GIT rev-parse --verify HEAD`"	&& HAVE_PACKAGE_GIT=1
	PACKAGE_GIT_STATUS="`cd "$_srcdir_abs" && $GIT status -s`"			&& HAVE_PACKAGE_GIT=1
    fi 2>/dev/null

    if [ "$HAVE_PACKAGE_GIT" = 1 ]; then
	echo "INFO: Summary of GIT metadata about the workspace '$_srcdir_abs':
	PACKAGE_GIT_ORIGIN:	$PACKAGE_GIT_ORIGIN
	PACKAGE_GIT_BRANCH:	$PACKAGE_GIT_BRANCH
	PACKAGE_GIT_TSTAMP:	$PACKAGE_GIT_TSTAMP
	PACKAGE_GIT_HASH_S:	$PACKAGE_GIT_HASH_S
	PACKAGE_GIT_HASH_L:	$PACKAGE_GIT_HASH_L"
	[ -n "$PACKAGE_GIT_STATUS" ] && echo \
"	PACKAGE_GIT_STATUS (short list of differences against committed repository):
$PACKAGE_GIT_STATUS"
	echo ""
    fi
}

showBuilderFlags() {
	echo "INFO: Summary of flags that influence this run of the '$0':
	BLDARCH (for bld/inst):	$BLDARCH
	CHECKOUTDIR workspace:	$CHECKOUTDIR
	BUILDSUBDIR (subdirs):	$BUILDSUBDIR
	DESTDIR (for install):	$DESTDIR
	MAKE command to use:	$MAKE"
	[ -n "$MAKE_OPTS" ] && echo \
"	 Common MAKE command options (for build/install/make explicit targets):	$MAKE_OPTS"
	[ -n "$MAKE_OPTS_PAR" ] && echo \
"	 Additional MAKE command options for optional parallel build phase:	$MAKE_OPTS_PAR"
	[ -n "$MAKE_OPTS_SEQ" ] && echo \
"	 Additional MAKE command options for reliable sequential build phase: 	$MAKE_OPTS_SEQ"

	echo \
"	NOPARMAKE toggle:	$NOPARMAKE	(* 'yes' == sequential only)
	 NCPUS (private var):	$NCPUS
	 NPARMAKES jobs:	$NPARMAKES
	WARNLESS_UNUSED:	$WARNLESS_UNUSED	(* 'yes' == skip warnings about unused)"
	[ -n "$CFLAGS" ] && echo \
"	 CFLAGS (the C compiler):	$CFLAGS"
	[ -n "$CXXFLAGS" ] && echo \
"	 CXXFLAGS (C++ compiler):	$CXXFLAGS"
	[ -n "$CPPFLAGS" ] && echo \
"	 CPPFLAGS (C/C++ preprocessor):	$CPPFLAGS"
	echo \
"	Requested action:	$1"

	[ $# -gt 1 ] && case "$1" in
	build*|install*|make*)
	    shift
	    echo \
"	 Requested target(s):	$@"
	    ;;
	esac

	echo ""
}

while [ $# -gt 0 ]; do
	case "$1" in
	    --warnless-unused)
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
	    --show-builder-flags)
		SHOW_BUILDER_FLAGS=yes
		shift
		;;
	    --show-repository-metadata|--show-repository-metadata-git|--show-git-metadata)
		SHOW_REPOSITORY_METADATA_GIT=yes
		shift
		;;
	    --verbose)
		SHOW_BUILDER_FLAGS=yes
		SHOW_REPOSITORY_METADATA_GIT=yes
		shift
		;;
	    *)	break ;;
	esac
done

### The flags can be set in environment rather than passed on command line
[ x"$WARNLESS_UNUSED" = xyes ] && suppressWarningsUnused

### This is the last flag-reaction in the stack of such
[ x"$SHOW_BUILDER_FLAGS" = xyes ] && showBuilderFlags "$@"
[ x"$SHOW_REPOSITORY_METADATA_GIT" = xyes ] && showGitFlags

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
    help|-help|--help|-h)
	usage
	exit 2
	;;
    *)	echo "ERROR: Unknown parameter '$1' for '$0'" >&2
	usage
	exit 2
	;;
esac
