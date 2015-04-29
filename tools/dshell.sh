#!/bin/sh

# dshell binary wrapper 

# TODO
# - usage

PATH="${BUILDSUBDIR}/:`dirname $0`/:`dirname $0`/..:${CHECKOUTDIR}/tools:$CHECKOUTDIR/Installation/usr/bin:/usr/bin:$PATH"
export PATH
dsh="`which dshell`"
if [ $? != 0 ]; then
        echo "FATAL: dshell binary not found in PATH='$PATH'" >&2
        exit 1
fi
echo "INFO: Using dshell binary '$dsh'" >&2

exec ${dsh} ipc://@/malamute 1000 mshell "$1" "$2"
