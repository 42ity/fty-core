#!/bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Tomas Halman <TomasHalman@eaton.com>
#
# Description: installs basic components we need

# NOTE: This script may be standalone, so we do not depend it on scriptlib.sh
SCRIPTDIR=$(dirname $0)
[ -z "$CHECKOUTDIR" ] && CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
[ -z "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
export CHECKOUTDIR BUILDSUBDIR

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

limit_packages_recommends() {
    echo "INFO: Tell APT to not install packages from 'Recommends' category"
    mkdir -p /etc/apt/apt.conf.d
    echo 'APT::Install-Recommends "false";' > \
       "/etc/apt/apt.conf.d/02no-recommends"
}

limit_packages_paths() {
    echo "INFO: Trying to influence DPKG to avoid certain large useless files"
# see also "dpkg --set-selections" in ci-setup-test-machine.sh
    mkdir -p /etc/dpkg/dpkg.cfg.d
    echo '# avoid installation of docs packages except ours
path-exclude=/usr/share/doc/*
path-include=/usr/share/doc/bios*
path-include=/usr/share/doc/core*
# we need to keep copyright files for legal reasons
path-include=/usr/share/doc/*/copyright
# manpages can stay, we install a few ourselves
path-exclude=/usr/share/groff/*
path-exclude=/usr/share/info/*
# lintian stuff is small, but really unnecessary
path-exclude=/usr/share/lintian/*
path-exclude=/usr/share/linda/*
' >> /etc/dpkg/dpkg.cfg.d/excludes
}

limit_packages_docs() {
    # try to deny installation of some hugely useless packages
    # tex-docs for example are huge (850Mb) and useless on a test container
    echo "INFO: Tell DPKG to not install some large documentation packages (may complain, don't worry)"
    for P in \
        docutils-doc libssl-doc python-docutils \
        texlive-fonts-recommended-doc \
        texlive-latex-base-doc \
        texlive-latex-extra-doc \
        texlive-latex-recommended-doc \
        texlive-pictures-doc \
        texlive-pstricks-doc \
    ; do
        apt-mark hold "$P" >&2
        echo "$P  purge"
    done | dpkg --set-selections
}

update_system() {
    # if debian
    curl http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -
    # curl http://obs.mbt.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -
    apt-get clean all
    apt-get update
    limit_packages_recommends
    limit_packages_paths
    apt-get -f -y --force-yes --fix-missing install
    limit_packages_docs
    apt-get -f -y --force-yes install devscripts sudo doxygen curl git python-mysqldb \
        cppcheck msmtp libtool cpp gcc autoconf automake m4 pkg-config equivs dh-make
    mk-build-deps --tool 'apt-get --yes --force-yes' --install $CHECKOUTDIR/obs/core.dsc
    # and just to be sure about these space-hungry beasts
    apt-get remove --purge \
        docutils-doc libssl-doc python-docutils \
        texlive-fonts-recommended-doc texlive-latex-base-doc texlive-latex-extra-doc \
        texlive-latex-recommended-doc texlive-pictures-doc texlive-pstricks-doc
}

update_system
