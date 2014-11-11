#!/bin/sh

update_system() {
    # if debian
    curl http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -
    curl http://obs.mbt.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -
    apt-get clean all
    apt-get update
    apt-get -f -y --force-yes --fix-missing install
    apt-get -f -y --force-yes install devscripts sudo doxygen curl
}

update_system
