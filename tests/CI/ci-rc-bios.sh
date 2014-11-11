#!/bin/sh -e

start(){
stop
   set -x
   cd ~/bin &&
   /bin/rm -rf ~/simple.log
   nohup ./simple >~/simple.log 2>&1 &
   sleep 5
   pidof simple && pidof netmon
}

stop(){
   killall simple 2>/dev/null || true
   sleep 1
   killall netmon 2>/dev/null || true
}

usage(){
    echo "usage: $(basename $0) [options]"
}

if [ "x$1" == "x" ] ; then
    usage
    exit 1
fi

while [ "x$1" != "x" ] ; do
    case "$1" in
        -h|--help)
            usage
            exit 1
            ;;
        --start)
            OPERATION=start
            shift
            ;;
        --stop)
            OPERATION=stop
            shift
            ;;
        *)
            echo "Invalit option $1" 1>&2
            exit 1
            ;;
    esac
done

case "$OPERATION" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    *)
        echo "invalid operation/not specified"
        exit 1
        ;;
esac
