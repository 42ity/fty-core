#!/bin/bash
#
#   Copyright (c) 2014 - 2020 Eaton
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
#! \file    builder.sh
#  \brief   Script to generate all required files from fresh git checkout
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Script to generate all required files from fresh
#           git checkout and automate the fastest possible build (parallel
#           for speed, then sequential for reliability).
#           Supports automation of an "in-place" build in the current directory,
#           a "relocated" build in a sub-directory, and of a "distcheck" build.
#  \note    It expects to be run from the root of the project directory
#           (probably the checkout directory, unless you use strange set-ups)
#           i.e. as "./tools/builder.sh" or, for simplicity, by passing its
#           supported command-line parameters to the main "autogen.sh" script.

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

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

math_bash() {
	eval echo '$'"(($@))"
}

math_bc() {
	echo "$@" | bc
}

math_expr() {
	expr "$@"
}

math_func=""
math_detect() {
    for I in math_bash math_expr math_bc ; do
	[ -z "$math_func" ] && N="`eval $I 5 + 1 2>/dev/null`" && \
	[ "$N" = 6 ] && math_func="$I"
    done
    [ -n "$math_func" ]
}
math_detect
#echo "=== math_func='$math_func'"

do_math() {
    [ -n "$math_func" ] && $math_func "$@"
}

incr() {
    # Parameter: name of variable to increment
    eval $1="`eval $math_func '$'$1 + 1`" || eval $1=""
}

VERB_COUNT=0
verb_run() {
	incr VERB_COUNT || VERB_COUNT=""

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
# gather and calculate this information. Note that "psrinfo" count is not
# an honest approach (there may be limits of current CPU set etc.) but is
# a better upper bound than nothing...
[ x"$NCPUS" = x ] && { \
    NCPUS="`/usr/bin/getconf _NPROCESSORS_ONLN`" || \
    NCPUS="`/usr/bin/getconf NPROCESSORS_ONLN`" || \
    NCPUS="`cat /proc/cpuinfo | grep -wc processor`" || \
    { [ -x /usr/sbin/psrinfo ] && NCPUS="`/usr/sbin/psrinfo | wc -l`"; } \
    || NCPUS=1; } 2>/dev/null
[ x"$NCPUS" != x -a "$NCPUS" -ge 1 ] || NCPUS=1
[ x"$NPARMAKES" = x ] && { NPARMAKES="`do_math "$NCPUS" '*' 2`" || NPARMAKES=2; }
[ x"$NPARMAKES" != x -a "$NPARMAKES" -ge 1 ] || NPARMAKES=2
[ x"$MAXPARMAKES" != x ] && [ "$MAXPARMAKES" -ge 1 ] && \
    [ "$NPARMAKES" -gt "$MAXPARMAKES" ] && \
    echo "INFO: Detected or requested NPARMAKES=$NPARMAKES," \
        "however a limit of MAXPARMAKES=$MAXPARMAKES was configured" && \
    NPARMAKES="$MAXPARMAKES"

# GNU make allows to limit spawning of jobs by load average of the host,
# where LA is (roughly) the average amount over the last {timeframe} of
# queued processes that are ready to compute but must wait for CPU.
[ x"$PARMAKE_LA_LIMIT" = x ] && PARMAKE_LA_LIMIT=4.0

# Just make sure this variable is defined
[ -z "$CONFIGURE_FLAGS" ] && CONFIGURE_FLAGS=""

# enable timing of the steps
case "$TIME_MAKE" in
    time|*bin/time)	;;
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	TIME_MAKE="time" ;;
    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]|"")
	TIME_MAKE="" ;;
    *)	echo "WARNING: Ingoring unrecognized value of TIME_MAKE='$TIME_MAKE'" >&2
	TIME_MAKE="" ;;
esac

case "$TIME_CONF" in
    time|*bin/time)	;;
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	TIME_CONF="time" ;;
    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee]|"")
	TIME_CONF="" ;;
    *)	echo "WARNING: Ingoring unrecognized value of TIME_CONF='$TIME_CONF'" >&2
	TIME_CONF="" ;;
esac

# Normalize the optional flags
case "$NOPARMAKE" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	NOPARMAKE=yes ;;
    *)	NOPARMAKE=no  ;;
esac

case "$NOSEQMAKE" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	NOSEQMAKE=yes ;;
    *)	NOSEQMAKE=no  ;;
esac

case "$OPTSEQMAKE" in
    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee])
	OPTSEQMAKE=no   ;;
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	OPTSEQMAKE=yes  ;;
    *)	OPTSEQMAKE=auto ;;
	# By default, don't require seqmake for certain targets
esac

case "$NODISTCLEAN" in
    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
	NODISTCLEAN=yes ;;
    *)	NODISTCLEAN=no  ;;
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
				echo "INFO: Makefile absent in '`pwd`, skipping '$MAKE $@' for a cleaning action"
#distclean?#			[ -d config ] && rm -rf config
				return 0 ;;
		    *)
				echo "ERROR: Makefile absent in '`pwd`', can not fulfill '$MAKE $@'" >&2
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

	verb_run $TIME_MAKE $MAKE "$@"; RES=$?

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

do_make_dc() {
    # Wrapper for "make distclean" after which we intend to go on
    # Ensure that ./configure still exists after this, if it did exist before
    [ "$NODISTCLEAN" = yes ] && \
	echo "INFO: distclean action disabled by user request" && \
	return 0

    REMAKE_CONFIGURE=n
    [ -s ./configure -a -x ./configure ] && REMAKE_CONFIGURE=y

    ( BUILDER_RETAIN_CONFIGURE=yes; export BUILDER_RETAIN_CONFIGURE
      do_make "$@" )
    RES=$?

    # If the script was there but disappeared, remake it
    # If there were some dependencies like Makefile.in or automake scripts
    # this also ensures they exist again
    if [ x"$REMAKE_CONFIGURE" = xy ]; then
	verb_run ./autogen.sh || exit
	[ -s ./configure -a -x ./configure ] || exit
    fi

    return $RES
}


do_build() {
	MRES=255

	MRES_P="SKIPPED"
	MRES_S="SKIPPED"

	echo "INFO-MAKE[$$]: `date`: beginning do_build() in directory '`pwd`'"
	if [ x"$NOPARMAKE" != xyes ]; then 
	    echo "=== PARMAKE[$$] (fast first pass which is allowed to fail): $MAKE_OPTS_PAR $MAKE_OPTS $@"
	    case " $MAKE_OPTS_PAR $MAKE_OPTS $*" in
		*\ V=*|*\ --trace*)
		    do_make $MAKE_OPTS_PAR $MAKE_OPTS -j $NPARMAKES -k -l $PARMAKE_LA_LIMIT "$@"
		    MRES=$? ;;
		*)
		    do_make V=0 $MAKE_OPTS_PAR -j $NPARMAKES -k -l $PARMAKE_LA_LIMIT "$@"
		    MRES=$? ;;
	    esac
	    MRES_P="$MRES"
	else
	    echo "=== PARMAKE[$$]: disabled by user request"
	fi

	if [ "$MRES_P" = 0 -a "$OPTSEQMAKE" = yes ]; then
	    echo "=== SEQMAKE[$$]: disabled by user request as optional" \
		"(only required if PARMAKE failed)"
	else
	# User can request 'builder.sh install-subdir V=0' or somesuch
	# to suppress the build tracing, or '... --trace' to increase it
	# ...or the MAKE variable can be overridden to the same effect
	    if [ x"$NOSEQMAKE" != xyes ]; then 
		echo "=== SEQMAKE[$$]: $MAKE_OPTS_SEQ $MAKE_OPTS $@"
		do_make $MAKE_OPTS_SEQ $MAKE_OPTS "$@"
		MRES=$?
		MRES_S="$MRES"
	    else
		echo "=== SEQMAKE[$$]: disabled by user request"
	    fi
	fi

	echo "INFO-MAKE[$$]: `date`: do_build() results: make '$@' : PARMAKE=$MRES_P SEQMAKE=$MRES_S overall=$MRES"
	echo ""

	return $MRES
}

buildSamedir() {
	do_make_dc -k distclean
	verb_run $TIME_CONF ./configure $CONFIGURE_FLAGS && \
	{ do_make_dc -k clean; do_build "$@"; }
}

buildSubdir() {
	do_make_dc -k distclean
	( { echo "INFO: (Re-)Creating the relocated build directory in '${BUILDSUBDIR}'..."
	  rm -rf "${BUILDSUBDIR}"; \
	  mkdir "${BUILDSUBDIR}" && \
	  cd "${BUILDSUBDIR}"; } && \
	verb_run $TIME_CONF "$CHECKOUTDIR/configure" $CONFIGURE_FLAGS && \
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

_WARN_FATAL=0
makeWarningsFatal() {
	[ "$_WARN_FATAL" != 0 ] && return
	CFLAGS="$CFLAGS -Werror"
	CXXFLAGS="$CXXFLAGS -Werror"
	CPPFLAGS="$CPPFLAGS -Werror"
	export CFLAGS CXXFLAGS CPPFLAGS
	echo "INFO: Fixed up CFLAGS, CXXFLAGS and CPPFLAGS to make reported warnings fatal"
	_WARN_FATAL=1
}

usage() {
	echo "Usage: $0"
	echo "		- without parameters does just a classic autogen.sh job"
	echo ""
	echo "Usage: $0 [--warnless-unused] [--warn-fatal|-Werror] \ "
	echo "    [--disable-parallel-make|--disable-sequential-make|--disable-distclean] \ "
	echo "    [--optional-sequential-make [yes|no|auto] ] [--parmake-la-limit X.Y] \ "
	echo "    [--show-timing|--show-timing-make|--show-timing-conf] \ "
	echo "    [--show-repository-metadata] [--verbose] \ "
	echo "    [--show-builder-flags] [--configure-flags '...'] \ "
	echo "    [--install-dir 'dirname'] [--build-subdir 'dirname'] \ "
	echo "    { build-samedir | build-subdir | install-samedir | install-subdir \ "
	echo "      | make-samedir | make-subdir } [maketargets...]"
	echo ""
	echo "Usage: $0 [--debug-makefile] \ "
	echo "	   { build*|install*|make*|conf* } [maketargets...]"
	echo ""
	echo "These modes (re-)create the configure script and optionally either just"
	echo "rebuild, or rebuild and install into a DESTDIR, or make the requested"
	echo "project targets. Note that the 'make' actions do not involve clearing and"
	echo "reconfiguring the build area. For output clarity you can avoid the parallel"
	echo "pre-build step with 'export NOPARMAKE=Y' or '--noparmake' flag, while the"
	echo "'--debug-makefile' flag quickly enables several options at once, including"
	echo "verbosity, -Werror, and enforced sequential builds to trace make failures."
	echo "The '--optional-sequential-make' (silent default: 'auto'; implicit value if"
	echo "only the flag was specified: 'yes') skips a seqmake if parmake succeeded;"
	echo "where 'auto' is like 'yes' only for some tasks like check or dist."
	echo "The '--parmake-la-limit' allows to set a floating-point limit of load average"
	echo "where parallel gmake would stop spawning jobs on this host (default: 4.0)"
	echo ""
	echo "Some special uses without further parameters (--options above are accepted):"
	echo "Usage: $0 distcheck [<list of configure flags>]"
	echo "		- execute the distclean, configure and make distcheck"
	echo "Usage: $0 configure [<list of configure flags>]"
	echo "		- execute the distclean and configure step and exit"
	echo "Usage: $0 configure-subdir [<list of configure flags>]"
	echo "		- execute the configure step in a freshly made subdir and exit"
	echo "Usage: $0 distclean"
	echo "		- execute the distclean step and exit"
	echo "Usage: $0 run-subdir cmd [args...]"
	echo "		- change into the build subdir and run the command"
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
	CONFIGURE_FLAGS:	$CONFIGURE_FLAGS
	MAKE command to use:	$MAKE"
	[ -n "$MAKE_OPTS" ] && echo \
"	 Common MAKE command options (for build/install/make explicit targets):	$MAKE_OPTS"
	[ -n "$MAKE_OPTS_PAR" ] && echo \
"	 Additional MAKE command options for optional parallel build phase:	$MAKE_OPTS_PAR"
	[ -n "$MAKE_OPTS_SEQ" ] && echo \
"	 Additional MAKE command options for reliable sequential build phase: 	$MAKE_OPTS_SEQ"

	echo \
"	NOSEQMAKE toggle:	$NOSEQMAKE	(* 'yes' == parallel only, if enabled)
	OPTSEQMAKE toggle:	$OPTSEQMAKE
	NOPARMAKE toggle:	$NOPARMAKE	(* 'yes' == sequential only, if enabled)
	 NCPUS (private var):	$NCPUS
	 NPARMAKES jobs:	$NPARMAKES
	 PARMAKE_LA_LIMIT:	$PARMAKE_LA_LIMIT
	NODISTCLEAN toggle:	$NODISTCLEAN
	WARNLESS_UNUSED:	$WARNLESS_UNUSED	(* 'yes' == skip warnings about unused)
	WARN_FATAL:		$WARN_FATAL"
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

	echo \
"	Measure TIME_MAKE:	$TIME_MAKE
	Measure TIME_CONF:	$TIME_CONF
"
}

# Note that this loop processes optional "--options" before going on to the
# actions below. We must "shift" explicitly on every option, and then we
# fall through on an unknown keyword - considering it a potential option.
while [ $# -gt 0 ]; do
	case "$1" in
	    --build-subdir|--build-dir)
		BUILDSUBDIR="$2"
		shift 2
		;;
	    --install-dir)
		DESTDIR="$2"
		export DESTDIR
		shift 2
		;;
	    --configure-flags)
		CONFIGURE_FLAGS="$2"
		shift 2
		;;
	    --warnless-unused)
		WARNLESS_UNUSED=yes
		shift
		;;
	    -Werror|--warn-fatal)
		WARN_FATAL=yes
		shift
		;;
	    --nodistclean|--disable-distclean|--no-distclean)
		NODISTCLEAN=yes
		shift
		;;
	    --optseqmake|--optional-sequential-make)
		case "$2" in
		    [Yy]|[Yy][Ee][Ss]|[Oo][Nn]|[Tt][Rr][Uu][Ee])
			OPTSEQMAKE=yes ; shift ;;
		    [Nn]|[Nn][Oo]|[Oo][Ff][Ff]|[Ff][Aa][Ll][Ss][Ee])
			OPTSEQMAKE=no ; shift ;;
		    [Aa][Uu][Tt][Oo])
			OPTSEQMAKE=auto ; shift ;;
		    *)  OPTSEQMAKE=yes ;; # Default for standalone keyword
		esac
		shift
		;;
	    --noparmake|--disable-parallel-make|--no-parmake)
		NOPARMAKE=yes
		shift
		;;
	    --parmake|--enable-parallel-make)
		NOPARMAKE=no
		shift
		;;
	    --parmake-la-limit)
		PARMAKE_LA_LIMIT="$2"
		shift 2
		;;
	    --noseqmake|--disable-sequential-make|--no-seqmake)
		NOSEQMAKE=yes
		shift
		;;
	    --seqmake|--enable-sequential-make)
		NOSEQMAKE=no
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
	    --show-timing-make)
		TIME_MAKE=time
		shift
		;;
	    --show-timing-conf|--show-timing-configure)
		TIME_CONF=time
		shift
		;;
	    --show-timing|--show-timings)
		TIME_MAKE=time
		TIME_CONF=time
		shift
		;;
	    --verbose)
		SHOW_BUILDER_FLAGS=yes
		SHOW_REPOSITORY_METADATA_GIT=yes
		TIME_MAKE=time
		TIME_CONF=time
		shift
		;;
	    --debug-makefile)
		# A special mode to debug makefiles themselves, minimal cruft
		SHOW_BUILDER_FLAGS=yes
		SHOW_REPOSITORY_METADATA_GIT=yes
		WARNLESS_UNUSED=yes
		WARN_FATAL=yes
		TIME_MAKE=time
		TIME_CONF=time
		# Enforce first a sequential build with little verbosity and
		# linear output
		NOPARMAKE=no
		NPARMAKES=1
		OPTSEQMAKE=no
		[ -z "$MAKE_OPTS_SEQ" ] && MAKE_OPTS_SEQ="V=1 --trace"
		shift
		;;
	    *)	break ;;
	esac
done

### The flags can be set in environment rather than passed on command line
[ x"$WARNLESS_UNUSED" = xyes ] && suppressWarningsUnused
[ x"$WARN_FATAL" = xyes ] && makeWarningsFatal

### This is the last flag-reaction in the stack of such
[ x"$SHOW_BUILDER_FLAGS" = xyes ] && showBuilderFlags "$@"
[ x"$SHOW_REPOSITORY_METADATA_GIT" = xyes ] && showGitFlags

### TODO: This currently grabs ALL actions and optional make-targets
### to make a decision... move the logic elsewhere to act per-target?
### consider e.g. "./autogen.sh make clean check all install"
[ "$OPTSEQMAKE" = auto ] && case "$*" in
    *check*|*test*|*dist*|*conf*|*clean*|*install*)
	echo "INFO: Switching from OPTSEQMAKE=auto to OPTSEQMAKE=yes due to chosen actions and/or targets: $*"
	OPTSEQMAKE=yes
	;;
esac

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
	### No "do_make_dc" wrapper for the explicit request for distclean
	verb_run $TIME_CONF ./configure && \
	do_make -k distclean
	;;
    distcheck)
	shift
	do_make_dc -k distclean
	verb_run $TIME_CONF ./configure $CONFIGURE_FLAGS "$@" && \
	do_make distcheck
	;;
    conf|config|configure)
	shift
	do_make_dc -k distclean
	verb_run $TIME_CONF ./configure $CONFIGURE_FLAGS "$@"
	;;
    conf-subdir|config-subdir|configure-subdir)
	shift
	do_make_dc -k distclean
	{ echo "INFO: (Re-)Creating the relocated build directory in '${BUILDSUBDIR}'..."
	  rm -rf "${BUILDSUBDIR}"; \
	  mkdir "${BUILDSUBDIR}" && \
	  cd "${BUILDSUBDIR}"; } && \
	verb_run $TIME_CONF "$CHECKOUTDIR/configure" $CONFIGURE_FLAGS "$@"
	;;
    run-subdir)
	shift
	if [ $# -le 0 -o ! -d "${BUILDSUBDIR}" ]; then
	    echo "FAIL: Cannot run '$@' under build dir '${BUILDSUBDIR}'" >&2
	    exit 2
	else
            _PROG="$1"
            shift
            case "$_PROG" in
                /*) ;;
                *)  # See if we have variants for requested relative pathname
                    if  [ ! -x "`pwd`/$_PROG" ] && \
                        [ -x "${BUILDSUBDIR}/$_PROG" ] \
                    ; then
                        _PROG="${BUILDSUBDIR}/$_PROG"
                        echo "INFO: Using program from '${BUILDSUBDIR}/'"
                    else
                        [ -x "`pwd`/$_PROG" ] && \
                            echo "INFO: Using program from '`pwd`/'" && \
                            _PROG="`pwd`/$_PROG"
                    fi
                    ;;
            esac

            case "$_PROG" in
                /*) ;;
                ./*) _PROG="`pwd`/$_PROG"; echo "INFO: Using program from '`pwd`/'" ;;
                *) echo "INFO: Using program from '${BUILDSUBDIR}/'" ;;
            esac

	    echo "INFO: Running '$_PROG $@' under build directory '${BUILDSUBDIR}'..."
	    ( cd "${BUILDSUBDIR}" && \
	      verb_run "$_PROG" "$@" )
	fi
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

