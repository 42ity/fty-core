#!/bin/sh

install_packages() {
    # if debian
    apt-get update
    apt-get -f -y --force-yes --fix-missing install
    apt-get -f -y --force-yes install "$@"
}

if [ "$1" != "" ] ; then
   install_packages "$@"
else
   echo "usage: $(basename $0) package [package ...]"
fi
