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
#! \file   ci-systemctl.sh
#  \brief  Tests if the systemctl wrapper script and REST API know about
#          daemon (non-timer) services that this system actually knows
#          among bios.target components
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
# TODO: Add REST API equivalent (currently it also has a filter - should
# not be needed after the wrapper script becomes THE ONLY filter for this).

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no NEED_CHECKOUTDIR=yes determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to run the systemctl tests"

usage(){
    echo "Usage: $(basename $0) [servicename...]
    servicename(s...)    List of systemd units to check, otherwise looks
                         up components of bios.target on this system
    --help|-h    print this help"
}

SERVICES=""
SERVICES_EXT="malamute"

# Regex of services for which systemctl must say 'Action not allowed'
[ -z "${MUST_FILTER-}" ] && \
    MUST_FILTER='^(bios-ssh-last-resort|biostimer-compress-logs|biostimer-verify-fs|ifplug-dhcp-autoconf|ipc-meta-setup)(|\.service|\.timer)$'

while [ $# -gt 0 ] ; do
    case "${1}" in
        -h|--help)
            usage
            exit 0
            ;;
        -*)
            echo "Invalid option $1" 1>&2
            usage
            exit 1
            ;;
        *)  SERVICES="$SERVICES $1" ;;
    esac
    shift
done

# Note: this default log filename will be ignored if already set by caller
# ERRCODE is maintained by settraps()
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'exit_summarizeTestlibResults $ERRCODE'

if [ -z "$SERVICES" ]; then
    test_it "GetRuntimeListOfServices"
    logmsg_info "Getting run-time list of services that bios.target 'Wants'"
    if isRemoteSUT ; then
        SERVICES="$(sut_run 'ls -1 /etc/systemd/system/bios.target.wants/*.service | while read F ; do echo "`basename "$F"`"; done | tr "\n" " "' )" || SERVICES=""
    else
        SERVICES="$(ls -1 /etc/systemd/system/bios.target.wants/*.service | while read F ; do echo "`basename "$F"`"; done | tr "\n" " ")" || SERVICES=""
    fi
    [ -z "$SERVICES" ] && die "Could not detect run-time list of 42ity services"
    print_result 0
    SERVICES="$SERVICES_EXT $SERVICES"
fi
logmsg_debug "Will inspect these service names: $SERVICES"

PATH="/usr/libexec/fty:/usr/share/fty/scripts:/usr/libexec/bios:/usr/share/bios/scripts:/usr/local/bin:/usr/local/sbin:/usr/bin:/usr/sbin:/bin:/sbin"
if ! isRemoteSUT ; then
    [ -n "$CHECKOUTDIR" ] && [ -d "$CHECKOUTDIR" ] && \
        PATH="$CHECKOUTDIR/tools:$PATH"
    [ -n "$BUILDSUBDIR" ] && [ -d "$BUILDSUBDIR" ] && \
        PATH="$BUILDSUBDIR/tools:$PATH"
fi

for S in $SERVICES; do
    RES=0
    FAILMSG=""
    test_it "systemctl_wrapper::$S"

    # NOTE: We do not care much for non-zero exit codes because services
    # can be stopped (3). Our question is if we may inspect them at all!
    if isRemoteSUT ; then
        OUT="$(sut_run 'PATH='"${PATH}"' systemctl is-active "'"$S"'" 2>&1')"
    else
        OUT="$(systemctl is-active "$S" 2>&1)"
    fi

    if [ -z "$OUT" ]; then
        logmsg_error "Query for '$S' returned empty output"
        FAILMSG="Empty_Output"
        RES=2
    else
        if [ -n "${MUST_FILTER-}" ] && [[ "$S" =~ $MUST_FILTER ]]; then
            echo "$OUT" | grep -v 'Action not allowed' >/dev/null && \
            RES=3 && \
            logmsg_error "Query for '$S' did not return 'Action not allowed' while we expect it to be forbidden (bad filter in wrapper script)" && \
            FAILMSG="Action_must_not_be_allowed"
        else # Service is not required filtered-away
            echo "$OUT" | grep 'Action not allowed' >/dev/null && \
            RES=1 && \
            logmsg_error "Query for '$S' returned 'Action not allowed' (bad filter in wrapper script)" && \
            FAILMSG="Action_not_allowed"
        fi
    fi
    print_result "$RES" "$FAILMSG"
done
