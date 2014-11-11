#/!bin/sh

set -u
set -e

wait_for_web() {
    for a in $(seq 60) ; do
        sleep 5
        if ( netstat -tan | grep 8000 >/dev/null ) ; then
            return
        fi
    done
}


# prepase environment
  # might have some mess
  killall tntnet 2>/dev/null || true
  # make sure sasl is running
  systemctl restart saslauthd
  # check sals is workung
  testsaslauthd -u bios -p @PASSWORD@ -s bios

# do the webserver
  cd core
  # make clean
  make web-test &
  MAKEPID=$!
  wait_for_web

# do the test
set +e
echo "============================================================"
/bin/bash tests/CI/test_web.sh
RESULT=$?
echo "============================================================"

# cleanup
kill $MAKEPID 2>/dev/null
sleep 2
killall tntnet 2>/dev/null
sleep 2
exit $RESULT
