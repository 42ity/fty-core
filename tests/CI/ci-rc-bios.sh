#!/bin/sh
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
#! \file   ci-rc-bios.sh
#  \brief  starts or stops the $BIOS daemons installed in $HOME
#  \author Tomas Halman <TomasHalman@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# do not set NEED_BUILDSUBDIR=yes because this aborts on unbuilt project
determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

# Names of daemons to (build and) start up for the test
# Currently we exclude "server-agent" because it spams a lot, unfiltered ;)
DAEMONS="`sed -n 's|ExecStart=@libexecdir@/@PACKAGE@/||p' "$CHECKOUTDIR"/systemd/bios-*.service.in | egrep -v 'server-agent|db-init|bios-networking'`"

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
    sleep 2
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
        sleep 2
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
        nohup "${prefix}/${1}" > ${BUILDSUBDIR}/${1}.log 2>&1 &
        sleep 5
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

stop() {
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && killall $d lt-$d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && pkill $d lt-$d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && kill `pidof $d lt-$d` 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && killall -KILL $d lt-$d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && pkill -KILL $d lt-$d 2>/dev/null ) || true
    done
    sleep 1
    for d in $DAEMONS ; do
       ( pidof $d lt-$d >/dev/null 2>&1 && kill -KILL `pidof $d lt-$d` 2>/dev/null ) || true
    done
    sleep 1
    # Test successful kills
    for d in $DAEMONS ; do
        pidof $d lt-$d >/dev/null 2>&1 && \
            echo "ERROR: stop(): $d still running (`pidof $d lt-$d`)" && return 1
    done
    echo "INFO: stop(): none of the DAEMONS ($DAEMONS) are running (OK)"
    return 0
}

status() {
    GOODSTATE="$1"
    [ -z "$GOODSTATE" ] && GOODSTATE=started
    RESULT=0
    for d in malamute $DAEMONS ; do
        echo -n "$d is currently "
        if pidof $d lt-$d >/dev/null 2>&1 ; then
            echo "running (`pidof $d lt-$d `)"
            [ "$GOODSTATE" = started ] || RESULT=1
        else
            echo "stopped"
            [ "$GOODSTATE" = stopped ] || RESULT=1
        fi
    done
    return $RESULT
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

start() {
    for d in $DAEMONS ; do
        start_daemon $d
    done
}

usage(){
    echo "Usage: $(basename $0) [options...]
Options:
    --stop       stop BIOS processes
    --start      start BIOS processes (does restart if BIOS is running)
    --status     check whether all processes are running
    --statusX    check whether all processes are stopped
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
        --stop)
            OPERATION=stop
            ;;
        --status)
            OPERATION=status
            ;;
        --statusX)
            OPERATION=statusX
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

case "$OPERATION" in
    start)
        RESULT=0
        stop
        update_compiled
        start_malamute && \
        start
        status started || \
            { echo "ERROR: Some daemons are not running" ; RESULT=1; }
        exit $RESULT
        ;;
    stop)
        RESULT=0
        stop || RESULT=$?
        stop_malamute || RESULT=$?
        status stopped || \
            { echo "ERROR: Some daemons are still running" ; RESULT=1; }
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
exit 1
