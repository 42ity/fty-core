#!/bin/sh

# dshell binary wrapper 

# TODO
# - usage

dsh=`dirname $0`/dshell
# exported by CI tests
[ -x "${BUILDSUBDIR}/dshell" ] && \
    dsh="${BUILDSUBDIR}/dshell"

exec ${dsh} ipc://@/malamute 1000 mshell "$1" "$2"
