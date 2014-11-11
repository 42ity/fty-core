#!/bin/sh

set -e

apt-get update
mk-build-deps --tool 'apt-get --yes --force-yes' --install ~/core/obs/core.dsc
cd ~/core

echo "======================== make and make check ==============================="
autoreconf -vfi && ./configure --prefix=$HOME && make && make check && make install

echo "===================== make dist and make distcheck ========================="
make distclean && ./configure && make dist && make distcheck
