#/!bin/sh
#
# Copyright (C) 2014-2015 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   ci-test-restapi.sh
#  \brief  sets up the sandbox and runs the tests of REST API for the $BIOS project.
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
# Include these explicitly since we don't use weblib.sh here
#. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR' (it may be empty but should exist)"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build, and BUILDSUBDIR='$BUILDSUBDIR' to run the REST API webserver"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

# Set up weblib test engine preference defaults for automated CI tests
[ -z "$WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT" ] && \
    WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT="fatal"
[ -z "$WEBLIB_QUICKFAIL" ] && \
    WEBLIB_QUICKFAIL=no
[ -z "$WEBLIB_CURLFAIL" ] && \
    WEBLIB_CURLFAIL=no
[ -z "$SKIP_NONSH_TESTS" ] && \
    SKIP_NONSH_TESTS=yes
export WEBLIB_CURLFAIL_HTTPERRORS_DEFAULT WEBLIB_QUICKFAIL WEBLIB_CURLFAIL SKIP_NONSH_TESTS

PATH="/usr/lib/ccache:/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

# Simple check for whether sudo is needed to restart saslauthd
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

usage(){
    echo "Usage: $(basename $0) [options...] [test_name...]"
    echo "options:"
    echo "  -u|--user   username for SASL (Default: '$BIOS_USER')"
    echo "  -p|--passwd password for SASL (Default: '$BIOS_PASSWD')"
    echo "  -s|--service service for SASL/PAM (Default: '$SASL_SERVICE')"
}

while [ $# -gt 0 ] ; do
    case "$1" in
        --user|-u)
            BIOS_USER="$2"
            shift 2
            ;;
        --passwd|-p)
            BIOS_PASSWD="$2"
            shift 2
            ;;
	--service|-s)
	    SASL_SERVICE="$2"
	    shift 2
	    ;;
        --help|-h)
            usage
            exit 1
            ;;
        *)  # Assume that list of test names follows
            # (positive or negative, see test_web.sh)
            break
            ;;
    esac
done

set -u
set -e

test_web_port() {
    netstat -tan | grep -w "${SUT_WEB_PORT}" | egrep 'LISTEN' >/dev/null
}

test_web_process() {
    [ -z "$MAKEPID" ] && return 0

    if [ ! -d /proc/$MAKEPID ]; then
        logmsg_error "Web-server process seems to have died!" >&2
        # Ensure it is dead though, since we abort the tests now
        kill $MAKEPID >/dev/null 2>&1
        wait $MAKEPID >/dev/null 2>&1
        return
    fi
    return 0
}

wait_for_web() {
    for a in $(seq 60) ; do
        sleep 5
        test_web_process || exit
        if ( test_web_port ) ; then
            return 0
        fi
    done
    logmsg_error "Port ${SUT_WEB_PORT} still not in LISTEN state" >&2
    return 1
}

MAKEPID=""
DBNGPID=""
kill_daemons() {
    if [ -n "$MAKEPID" -a -d "/proc/$MAKEPID" ]; then
        logmsg_info "Killing make web-test PID $MAKEPID to exit"
        kill -INT "$MAKEPID"
    fi
    if [ -n "$DBNGPID" -a -d "/proc/$DBNGPID" ]; then
        logmsg_info "Killing agent-dbstore PID $DBNGPID to exit"
        kill -INT "$DBNGPID"
    fi

    killall -INT tntnet agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1
    killall      tntnet agent-dbstore lt-agent-dbstore 2>/dev/null || true; sleep 1

    ps -ef | grep -v grep | egrep "tntnet|agent-dbstore" | egrep "^`id -u -n` " && \
        ps -ef | egrep -v "ps|grep" | egrep "$$|make" && \
        logmsg_error "tntnet and/or agent-dbstore still alive, trying SIGKILL" && \
        { killall -KILL tntnet agent-dbstore lt-agent-dbstore 2>/dev/null ; exit 1; }

    return 0
}

# prepare environment
  # might have some mess
  killall tntnet lt-agent-dbstore agent-dbstore 2>/dev/null || true
  sleep 1
  killall -KILL tntnet lt-agent-dbstore agent-dbstore 2>/dev/null || true
  sleep 1
  test_web_port && \
    die "Port ${SUT_WEB_PORT} is in LISTEN state when it should be free"

  # make sure sasl is running
  if ! $RUNAS systemctl --quiet is-active saslauthd; then
    logmsg_info "Starting saslauthd..."
    $RUNAS systemctl start saslauthd || \
      [ x"$RUNAS" = x ] || \
      logmsg_warn "Could not restart saslauthd, make sure SASL and SUDO" \
        "are installed and /etc/sudoers.d/bios_01_citest is set up per INSTALL docs"
  fi
  # check SASL is working
  logmsg_info "Checking local SASL Auth Daemon"
  testsaslauthd -u "$BIOS_USER" -p "$BIOS_PASSWD" -s "$SASL_SERVICE" && \
    logmsg_info "saslauthd is responsive and configured well!" || \
    logmsg_error "saslauthd is NOT responsive or not configured!" >&2

  # make sure message bus is running
  if ! $RUNAS systemctl --quiet is-active malamute; then
    logmsg_info "Starting malamute..."
    $RUNAS systemctl start malamute || \
      [ x"$RUNAS" = x ] || \
      logmsg_warn "Could not restart malamute, make sure SASL and SUDO" \
        "are installed and /etc/sudoers.d/bios_01_citest is set up per INSTALL docs"
  fi

  # make sure database is running
  if ! $RUNAS systemctl --quiet is-active mysql; then
    logmsg_info "Starting mysql..."
    $RUNAS systemctl start mysql || \
      [ x"$RUNAS" = x ] || \
      logmsg_warn "Could not restart mysql, make sure SASL and SUDO" \
        "are installed and /etc/sudoers.d/bios_01_citest is set up per INSTALL docs"
  fi

# do the webserver
  LC_ALL=C
  LANG=C
  export BIOS_USER BIOS_PASSWD SASL_SERVICE LC_ALL LANG
  logmsg_info "Ensuring files for web-test exist and are up-to-date..."

  if [ ! -x "${BUILDSUBDIR}/config.status" ]; then
    logmsg_warn "Did not detect ${BUILDSUBDIR}/config.status, so will try to configure the project first, now..."
    ./autogen.sh --nodistclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG} || exit
  fi
  ./autogen.sh ${AUTOGEN_ACTION_MAKE} V=0 web-test-deps || exit
  ./autogen.sh ${AUTOGEN_ACTION_MAKE} V=0 web-test-deps-inst || \
    logmsg_warn "BIOS-1262: Could not install required scripts, password-related REST API tests will likely fail"

  logmsg_info "Spawning the web-server in the background..."
  ./autogen.sh --noparmake ${AUTOGEN_ACTION_MAKE} web-test &
  MAKEPID=$!

  # TODO: this requirement should later become the REST AGENT
  logmsg_info "Spawning the agent-dbstore server in the background..."
  ${BUILDSUBDIR}/agent-dbstore &
  DBNGPID=$!

  # Ensure that no processes remain dangling when test completes
  # The ERRCODE is defined by settraps() as the program exitcode
  # as it enters the trap
  TRAP_SIGNALS=EXIT settraps 'echo "CI-EXIT: $0: test finished (up to the proper exit command)..." >&2; kill_daemons'
  TRAP_SIGNALS="HUP INT QUIT TERM" settraps '[ "$ERRCODE" = 0 ] && ERRCODE=123; echo "CI-EXIT: $0: got signal, aborting test..." >&2; kill_daemons && exit $ERRCODE'

  logmsg_info "Waiting for web-server to begin responding..."
  wait_for_web && \
    logmsg_info "Web-server is responsive!" || \
    logmsg_error "Web-server is NOT responsive!" >&2
  logmsg_info "Waiting for webserver process $MAKEPID to settle after startup..."
  sleep 5
  test_web_process || exit

test_web() {
    echo "==== Calling test_web.sh ==================================="
    /bin/bash "${CHECKOUTDIR}"/tests/CI/test_web.sh -u "$BIOS_USER" -p "$BIOS_PASSWD" -s "$SASL_SERVICE" "$@"
    RESULT=$?
    echo "==== test_web RESULT: ($RESULT) =================================="
    return $RESULT
}

ci_loaddb_default() {
    echo "--------------- reset db: default ----------------"
    for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DATA" "$DB_DATA_TESTREST"; do
        loaddb_file "$data" || exit $?
    done
    return 0
}

test_web_default() {
    ci_loaddb_default && \
    test_web "$@"
}

test_web_topo_p() {
    echo "----------- reset db: topology : power -----------"
    for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_TOPOP"; do
        loaddb_file "$data" || exit $?
    done
    test_web "$@"
}

test_web_topo_l() {
    echo "---------- reset db: topology : location ---------"
    for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_TOPOL"; do
        loaddb_file "$data" || exit $?
    done
    test_web "$@"
}

asset_create() {
    echo "---------- reset db: asset : create ---------"
    for data in "$DB_BASE" "$DB_DATA"; do
          loaddb_file "$data" || exit $?
    done
    test_web "$@"
}

# Try to accept the BIOS license on server
SKIP_SANITY=yes test_web 00_license-CI-forceaccept.sh.test || \
    logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

# do the test
set +e
if [ $# = 0 ]; then
    test_web_default -topology_power
    RESULT=$?
    test_web_process || exit
    if [ "$RESULT" -eq 0 ]; then
        test_web_topo_p topology_power
        RESULT=$?
    fi
    test_web_process || exit
else
    # selective test routine
    while [ $# -gt 0 ]; do
        case "$1" in
            topology_power*)
                test_web_topo_p "$1"
                RESULT=$? ;;
            asset_create*)
                asset_create $1
                RESULT=$? ;;
            *)        test_web_default "$1"
                RESULT=$? ;;
        esac
        shift
        test_web_process || exit
        [ "$RESULT" != 0 ] && break
    done
fi
ci_loaddb_default

# cleanup
kill $MAKEPID >/dev/null 2>&1 || true
sleep 2
killall tntnet >/dev/null 2>&1 || true
sleep 2

if [ "$RESULT" = 0 ]; then
    logmsg_info "Overall result: SUCCESS"
else
    logmsg_error "Overall result: FAILED ($RESULT) seek details above"
fi

exit $RESULT
