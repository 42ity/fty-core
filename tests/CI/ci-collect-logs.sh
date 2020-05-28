#!/bin/bash

# Copyright (C) 2014 - 2020 Eaton
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
#! \file   ci-collect-logs.sh
#  \brief  Downloads log files from VM and prints them on stdout.
#  \brief  Supposed to be post build step.
#  \author Tomas Halman <TomasHalman@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

#
# defaults
#
SUT_SSH_PORT=22
SUT_USER="root"
SUT_HOST="$BUILDMACHINE"
SUT_IS_REMOTE=yes

usage() {
    echo "Usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine|--sut-host|-sh NAME    virtual machine host name [$SUT_HOST]"
    echo "    -p|--port|--sut-port-ssh|-sp PORT   virtual machine ssh port [$SUT_SSH_PORT]"
    echo "    -u|--user|--sut-user|-su USER       virtual machine ssh username [$SUT_USER]"
    echo "    -h|--help                           print this help"
}

log_list() {
    # Note: cppcheck.xml is listed and processed separately
    REMCMD='find . -type f -name '"'"'*.log'"'"' -o -name '"'"'*.trs'"'"' -o -wholename '"'"'*/tests/junit/*.xml'"'"' -o -name '"'"'*.out*'"'"' -o -name '"'"'*.err*'"'"' | grep -v cppcheck.xml'
    sut_run "$REMCMD"
}

cppcheck_list() {
    sut_run "find . -type f -name cppcheck.xml"
}

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|-sh|--machine|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        -p|-sp|--port|--sut-port|--sut-port-ssh)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        -u|-su|--user|--sut-user)
            SUT_SSH_USER="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 1
            ;;
        *)
            echo "Invalid switch $1"
            usage
            exit 1
            ;;
    esac
done

if [ ! "$SUT_HOST" ] ; then
    echo "Machine is not specified!"
    usage
    exit 1
fi

printf "\n\n\n\n======================== collecting log files ========================\n"
logmsg_info "SSHing to '${SUT_USER}@$SUT_HOST' port '$SUT_SSH_PORT' from '`id -n -u`@`hostname`:$_SCRIPT_STARTPWD'"

LOGS=$(log_list | wc -l)
if [ $LOGS = 0 ] ; then
    echo "no log files"
else
    log_list | while read file ; do
        echo "$file"
        scp -P "$SUT_SSH_PORT" "${SUT_USER}@$SUT_HOST:$file" ./ 
    done
fi

LOGS=$(cppcheck_list | wc -l)
if [ $LOGS == 1 ] ; then
    cppcheck_list | while read file ; do
        scp -P "$SUT_SSH_PORT" "${SUT_USER}@$SUT_HOST:$file" ./
        sut_run "/bin/rm -f \"$file\""
    echo $file
    done
fi

exit 0 # no reason to fail
