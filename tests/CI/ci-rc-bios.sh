#!/bin/bash
#
# Copyright (C) 2014-2016 Eaton
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
#! \file   ci-rc-bios.sh
#  \brief  starts or stops the 42ity daemons installed in $HOME
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# NOTE: The rest of usual initialization is below, call dependent on situation

usage(){
    echo "Usage: $(basename $0) [options...]
Options:
    --stop       stop 42ity processes
    --start      start 42ity processes (does restart if 42ity is running)
    --status     check whether all processes are running
    --statusX    check whether all processes are stopped properly
    --statusXX   check whether all processes are stopped or crashed at least
    --update-compiled   when using custom compiled code (rather than packaged)
                 use this option to ensure needed programs are up-to-date
                 (invoked automatically before a start)
    --help|-h    print this help"
}

OPERATION=help

while [ $# -gt 0 ] ; do
    case "${1}" in
        -h|--help)
            OPERATION=help
            ;;
        --start)
            OPERATION=start
            ;;
        --restart)
            OPERATION=restart
            ;;
        --startQ|--start-quick)
            OPERATION=startQ
            ;;
        --stop)
            OPERATION=stop
            ;;
        --status)
            OPERATION=status
            ;;
        --statusX)
            OPERATION=statusX
            ;;
        --statusXX)
            OPERATION=statusXX
            ;;
        --update-compiled)
            OPERATION=update_compiled
            ;;
        *)
            echo "Invalid option $1" 1>&2
            usage
            exit 1
            ;;
    esac
    shift
done

# For VTE or similar cases, just use the systemd integration and quit
if isRemoteSUT ; then

    SERVICES="$(sut_run 'ls -1 /etc/systemd/system/bios.target.wants/*.service | egrep -v "biostimer-|malamute|dc_th|db-init|bios-networking" | while read F ; do echo "`basename "$F"`"; done | tr "\n" " "' )"

    do_statusSVC() {
        GOODSTATE="$1"
        [ -z "$GOODSTATE" ] && GOODSTATE=started
        RESULT=0
        for s in malamute $SERVICES ; do
            echo -n "$s (remote) is currently "
            sut_run "/bin/systemctl status $s" >/dev/null 2>&1
            case $? in
            0) # Is running
                echo -n "running "
                [ "$GOODSTATE" = started ] && \
                    echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
                ;;
            3) # Oneshot exited (maybe OK), but is not running now
               # Ordinary service requested to stop and not running as well
                echo -n "stopped "
                sut_run "/bin/systemctl is-failed $s" >/dev/null 2>&1 && \
                    { [ "$GOODSTATE" = stoppedOrCrashed ] && \
                        { echo "and crashed [-WARN-]"; RESULT=0; } || \
                        { case "$s" in
#                            *kpi-uptime*) "and crashed [IGNORE]"; RESULT=0;; # BIOS-1910
                            *) echo "and crashed [-FAIL-]"; RESULT=3;;
                          esac; } ; } || \
                    { [ "$GOODSTATE" = stopped -o "$GOODSTATE" = stoppedOrCrashed ] && \
                        echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; } ; }
                ;;
            1|*) # Is not running or other error
                echo -n "stopped "
                [ "$GOODSTATE" = stopped -o "$GOODSTATE" = stoppedOrCrashed ] && \
                    echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
                ;;
            esac
        done
        return $RESULT
    }

    statusSVC() {
        GOODSTATE="$1"
        [ -z "$GOODSTATE" ] && GOODSTATE=started
        OUT="`do_statusSVC "$@" 2>&1`"
        RES=$?
        if [ $RES != 0 ] || [ "${CI_DEBUG-}" -gt 6 ] 2>/dev/null ; then
            echo "$OUT"
        fi
        [ "$RES" = 0 ] && echo "42ity services: OK (all remote $GOODSTATE)" || \
            echo "42ity services: FAILED (some remote not $GOODSTATE)"
        return $RES
    }

    case "$OPERATION" in
    start|restart|startQ)
        [ "$OPERATION" = startQ ] && OPERATION=start
        sut_run "/bin/systemctl $OPERATION malamute bios.service $SERVICES" && \
            statusSVC started
        exit $?
        ;;
    stop)

        # BIOS-1807: so far "bios-agent-autoconfig" service or its child process
        # "nut-scanner" tends to hang when stopping... so kill them ruthlessly!
        case "$SERVICES" in
            *agent-autoconfig*)
                sut_run '/bin/systemctl stop bios-agent-autoconfig & sleep 1 ; \
                ( pidof agent-autoconfig nut-scanner >/dev/null 2>&1 && echo "TERMING: agent-autoconfig nut-scanner" && kill -TERM `pidof agent-autoconfig nut-scanner` 2>/dev/null && sleep 3 ) || true
                ( pidof agent-autoconfig nut-scanner >/dev/null 2>&1 && echo "KILLING: agent-autoconfig nut-scanner" && kill -KILL `pidof agent-autoconfig nut-scanner` 2>/dev/null && sleep 1 ) || true
                wait'
                ;;
            *) ;; # Offender not running as a service, and was killed above if a daemon
        esac

        RES=255
        for i in $(seq 1 5) ; do
            [ "$i" -gt 1 ] && echo "Retrying to stop 42ity services (did #`expr $i - 1` attepmts so far)..." >&2
            sut_run "/bin/systemctl $OPERATION bios.service $SERVICES malamute" && \
                statusSVC stoppedOrCrashed
            RES=$?
            [ "$RES" = 0 ] && exit $RES
        done
        echo "FAILED($RES) to stop 42ity services after $i attempts!" >&2
        exit $RES
        ;;
    status)  # Good if all services are started
        statusSVC started
        exit
        ;;
    statusXX)  # Good if all services are stopped one way or another
        statusSVC stoppedOrCrashed
        exit
        ;;
    statusX)  # Good if all services are stopped properly
        statusSVC stopped
        exit
        ;;
    update_compiled)
        echo "WARN: update_compiled() is a no-op for remote testing"
        exit 0
        ;;
    help)
        usage
        exit 1
        ;;
    *)
        echo "ERROR: '$OPERATION' is not supported for remote testing" >&2
        exit 1
        ;;
    esac

    # Should not get here
    exit 255
fi

# do not set NEED_BUILDSUBDIR=yes because this aborts on unbuilt project
determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

# Names of "bios-core" daemons to (build and) start up for the test
# Services of interest are those that are provided by 42ity packages
# built from non-"core" repositories, and not by "bios-core" itself.
DAEMONS="`sed -n 's|ExecStart=@libexecdir@/@PACKAGE@/||p' "$CHECKOUTDIR"/systemd/bios-*.service.in | sed -e 's|^\([^ ]*\)\([ \t].*\)$|\1|' | egrep -v 'db-init|bios-networking'`"
SERVICES="$(ls -1 /etc/systemd/system/bios.target.wants/*.service | egrep -v 'biostimer-|malamute|dc_th|db-init|bios-networking|bios-agent-nut' | while read F ; do BF="`basename "$F"`"; [ -s "$CHECKOUTDIR/systemd/$BF.in" ] || echo "$BF"; done | tr '\n' ' ')"

if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "Cannot find $BUILDSUBDIR/config.status, using preinstalled system binaries..."
    export PATH="/usr/bin:/usr/lib:/usr/libexec:/usr/lib/bios:/usr/libexec/bios:$PATH"
else
    echo "Found $BUILDSUBDIR/config.status, using custom-built binaries..."
    echo "Search path: $CHECKOUTDIR, $PWD"
    export PATH="/usr/lib/ccache:${PWD}:$BUILDSUBDIR:$BUILDSUBDIR/tools:$CHECKOUTDIR:$CHECKOUTDIR/tools:~/bin:~/lib:~/libexec:~/lib/bios:~/libexec/bios:$PATH"
fi

# Simple check for whether sudo is needed to restart saslauthd
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 -o "$CURID" = root ] || RUNAS="sudo"

stop_malamute(){
    # NOTE: This likely needs execution via sudo if user is not root
    $RUNAS systemctl stop malamute || true
    sleep 1
    pidof malamute >/dev/null 2>&1 && return 1
    echo "INFO: stop(): malamute is not running (OK)"
    return 0
}

start_malamute(){
    # NOTE1: This likely needs execution via sudo if user is not root
    # NOTE2: This restarts malamute if it is running and config changed;
    # but the service would remain running if config is the same.
    # The two-step routine below is needed for privilege elevation
    # to change the file in /etc/malamute
    rm -f "/tmp/.malamute.$$.cfg" 2>/dev/null || true

    cat > /tmp/.malamute.$$.cfg <<[eof]
# Note: This file was regenerated `date` by $0
#   Apply to the whole broker
server
    timeout = 10000     #   Client connection timeout, msec
    background = 0      #   Run as background process
    workdir = /tmp      #   Working directory for daemon
    verbose = 0         #   Do verbose logging of activity?

#   Apply to the Malamute service
mlm_server
    echo = binding Malamute service to 'ipc://@/malamute'
    bind
        endpoint = ipc://@/malamute
[eof]
    RESULT=$?

    if [ "$RESULT" = 0 -a -d /etc/malamute ]; then
        echo -n 'systemd service unit: malamute '
        RESTART=n
        if [ -s /etc/malamute/malamute.cfg -a -s /tmp/.malamute.$$.cfg ]; then
            if diff -bu /etc/malamute/malamute.cfg /tmp/.malamute.$$.cfg | \
               egrep -v '^(\-\-\-|\+\+\+|[ @]|[\+\-]\#.*regenerated)' \
            ; then
                # "diff" found some lines other than timestamp
                RESTART=y
            fi
        else
            RESTART=y
        fi >/dev/null

        if [ "$RESTART" = n ]; then
            # Config is the same
            echo -n "(config unchanged) "
            pidof malamute >/dev/null 2>&1 || $RUNAS systemctl start malamute
            RESULT=$?
        else
            stop_malamute
            $RUNAS cp /tmp/.malamute.$$.cfg /etc/malamute/malamute.cfg && \
            $RUNAS systemctl start malamute
            RESULT=$?
        fi
        sleep 1
        pidof malamute || RESULT=$?
        # copy, start or pidof could fail by this point;
        # otherwise we have RESULT==0 from diff-clause or cp-execution
    fi

    if [ "$RESULT" != "0" ] ; then
        echo "ERROR: failed to (re)start malamute" >&2
    fi

    rm -f /tmp/.malamute.$$.cfg 2>/dev/null
    return $RESULT
}

start_daemon(){
    prefix=""
    if which ${1} >/dev/null 2>&1; then
        prefix="`which ${1}`"
        prefix="`dirname "$prefix"`"
    fi

    if [ -x "${prefix}/${1}" ] ; then
        /bin/rm -rf ${BUILDSUBDIR}/${1}.log
        case "${1}" in
            *agent-nut)
                # TODO: Re-read the service ExecStart for a particular arglist?
                if [ -s "$CHECKOUTDIR/src/agents/nut/mapping.conf" ]; then
                    nohup "${prefix}/${1}" "$CHECKOUTDIR/src/agents/nut/mapping.conf" > ${BUILDSUBDIR}/${1}.log 2>&1 &
                elif [ -s "/usr/share/bios/agent-nut/mapping.conf" ]; then
                    nohup "${prefix}/${1}" "/usr/share/bios/agent-nut/mapping.conf" > ${BUILDSUBDIR}/${1}.log 2>&1 &
                else
                    logmsg_warn "Starting 'agent-nut' with no arg would fail or even segfault"
                    nohup "${prefix}/${1}" > ${BUILDSUBDIR}/${1}.log 2>&1 &
                fi
                ;;
            *)  nohup "${prefix}/${1}" > ${BUILDSUBDIR}/${1}.log 2>&1 & ;;
        esac

        sleep 1
        echo -n "${prefix}/$1 "
        pidof ${1} lt-${1}
        RESULT=$?
        if [ "$RESULT" != "0" ] ; then
            echo "ERROR: start(): failed to start $1" >&2
            exit 1
        fi
    else
        echo "ERROR: start(): $1 is missing" >&2
        exit 1
    fi
}

do_stop() {
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && killall $d lt-$d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && pkill $d lt-$d 2>/dev/null && sleep 1 & ) || true
    done
    wait
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && kill `pidof $d lt-$d` 2>/dev/null && sleep 1 & ) || true
    done
    wait
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && killall -KILL $d lt-$d 2>/dev/null && sleep 1 & ) || true
    done
    wait
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && pkill -KILL $d lt-$d 2>/dev/null && sleep 1 & ) || true
    done
    wait
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && kill -KILL `pidof $d lt-$d` 2>/dev/null && sleep 1 & ) || true
    done
    wait

    # BIOS-1807: so far "bios-agent-autoconfig" service or its child process
    # "nut-scanner" tends to hang when stopping... so kill them ruthlessly!
    case "$SERVICES" in
        *agent-autoconfig*)
            s="agent-autoconfig"
            /bin/systemctl stop bios-$s &
            sleep 1
            ( pidof agent-autoconfig nut-scanner >/dev/null 2>&1 && echo "TERMING: agent-autoconfig nut-scanner" && kill -TERM `pidof agent-autoconfig nut-scanner` 2>/dev/null && sleep 3 ) || true
            ( pidof agent-autoconfig nut-scanner >/dev/null 2>&1 && echo "KILLING: agent-autoconfig nut-scanner" && kill -KILL `pidof agent-autoconfig nut-scanner` 2>/dev/null && sleep 1 ) || true
            wait
            ;;
        *) ;; # Offender not running as a service, and was killed above if a daemon
    esac

    for s in $SERVICES ; do
        /bin/systemctl stop $s || true
    done
    # Test successful kills
    RESULT=0
    for d in $DAEMONS ; do
        pidof $d lt-$d >/dev/null 2>&1 && \
            echo "ERROR: stop(): daemon $d still running (`pidof $d lt-$d`)" >&2 && RESULT=1
    done
    for s in $SERVICES ; do
        /bin/systemctl status $s >/dev/null 2>&1 && \
            echo "ERROR: stop(): service $s still running" && RESULT=1
    done
    [ "$RESULT" = 0 ] && \
        echo "INFO: stop() OK: none of the DAEMONS (`echo $DAEMONS | tr '\n' ' '`) and SERVICES ($SERVICES) are running"
    return $RESULT
}

stop() {
    RES=255
    for i in $(seq 1 5) ; do
        [ "$i" -gt 1 ] && echo "Retrying to stop 42ity services (did #`expr $i - 1` attepmts so far)..." >&2
        do_stop
        RES=$?
        [ "$RES" = 0 ] && return $RES
    done
    echo "FAILED($RES) to stop 42ity services after $i attempts!" >&2
    return $RES
}

do_status() {
    GOODSTATE="$1"
    [ -z "$GOODSTATE" ] && GOODSTATE=started
    RESULT=0
    for d in malamute $DAEMONS ; do
        echo -n "daemon $d is currently "
        if pidof $d lt-$d >/dev/null 2>&1 ; then
            echo "running (`pidof $d lt-$d `) "
            [ "$GOODSTATE" = started ] && \
                echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
        else
            echo -n "stopped "
            [ "$GOODSTATE" = stopped -o "$GOODSTATE" = stoppedOrCrashed ] && \
                echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
        fi
    done
    for s in $SERVICES ; do
        echo -n "service $s is currently "
        /bin/systemctl status $s >/dev/null 2>&1
        case "$?" in
        0) # Is running
            echo -n "running "
            [ "$GOODSTATE" = started ] && \
                echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
            ;;
        3) # Oneshot exited OK, but is not running now
           # Ordinary service requested to stop and not running as well
            echo -n "stopped "
            /bin/systemctl is-failed $s >/dev/null 2>&1 && \
                { [ "$GOODSTATE" = stoppedOrCrashed ] && \
                    { echo "and crashed [-WARN-]"; RESULT=0; } || \
                    { case "$s" in
#                        *kpi-uptime*) "and crashed [IGNORE]"; RESULT=0;; # BIOS-1910
                        *) echo "and crashed [-FAIL-]"; RESULT=3;;
                      esac; } ; } || \
                { [ "$GOODSTATE" = stopped -o "$GOODSTATE" = stoppedOrCrashed ] && \
                    echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; } ; }
            ;;
        1) # Is not running or some other error
            echo -n "stopped "
            [ "$GOODSTATE" = stopped -o "$GOODSTATE" = stoppedOrCrashed ] && \
                echo "[--OK--]" || { echo "[-FAIL-]"; RESULT=1; }
            ;;
        esac
    done
    return $RESULT
}

status() {
    GOODSTATE="$1"
    [ -z "$GOODSTATE" ] && GOODSTATE=started
    OUT="`do_status "$@" 2>&1`"
    RES=$?
    if [ $RES != 0 ] || [ "${CI_DEBUG-}" -ge "${CI_DEBUGLEVEL_RUN-}" ] 2>/dev/null ; then
        echo "$OUT"
    fi
    [ "$RES" = 0 ] && echo "42ity services: OK (all local $GOODSTATE)" || \
        echo "42ity services: FAILED (some local not $GOODSTATE)"
    return $RES
}

update_compiled() {
    if  [ -z "$BUILDSUBDIR" ] || \
        [ ! -d "$BUILDSUBDIR" -o ! -x "$BUILDSUBDIR/config.status" ]\
    ; then
        # Use system bins, nothing to compile
        return 0
    fi

    logmsg_info "Ensuring that the tested programs have been built and up-to-date"
    if [ ! -f "$BUILDSUBDIR/Makefile" ] ; then
        ./autogen.sh --nodistclean --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
        ${AUTOGEN_ACTION_CONFIG}
    fi
    ./autogen.sh --optseqmake ${AUTOGEN_ACTION_MAKE} \
        web-test-deps $DAEMONS
}

do_start() {
    # Each service's start can take a while including a sleep to see if it's ok
    # So run multiple starters in parallel and then see how each one ended up
    RESULT=0
    BGPIDS=""
    for d in $DAEMONS ; do
        start_daemon $d &
        BGPIDS="$BGPIDS $!"
    done

    for s in $SERVICES ; do
        /bin/systemctl start $s &
        BGPIDS="$BGPIDS $!"
    done

    for P in $BGPIDS ; do
        wait "$P" || RESULT=$?
    done

    return $RESULT
}

start() {
    OUT="`do_start "$@" 2>&1`"
    RES=$?
    if [ $RES != 0 ] || [ "${CI_DEBUG-}" -ge "${CI_DEBUGLEVEL_RUN-}" ] 2>/dev/null ; then
        echo "$OUT"
    fi
    [ "$RES" = 0 ] && echo "42ity services: OK (all local started)" || \
        echo "42ity services: FAILED (some local not started)"
    return $RES
}

case "$OPERATION" in
    start|restart|startQ)
        RESULT=0
        if [ "$OPERATION" != startQ ] ; then
            stop    # Legacy usage: part of "start", should be for "restart"
            update_compiled
        fi
        start_malamute && \
        start
        status started || \
            { echo "ERROR: Some daemons are not running" >&2 ; RESULT=1; }
        exit $RESULT
        ;;
    stop)
        RESULT=0
        stop || RESULT=$?
        stop_malamute || RESULT=$?
        status stoppedOrCrashed || \
            { echo "ERROR: Some daemons are still running" >&2 ; RESULT=1; }
        exit $RESULT
        ;;
    status)
        status started
        exit
        ;;
    statusX)
        status stopped
        exit
        ;;
    statusXX)
        status stoppedOrCrashed
        exit
        ;;
    update_compiled)
        update_compiled
        exit
        ;;
    help)
        usage
        exit 1
        ;;
esac

# Unknown operation
exit 255
