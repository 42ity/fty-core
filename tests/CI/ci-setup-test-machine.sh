#!/bin/bash
#
# Copyright (C) 2014 Eaton
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
#! \file   ci-setup-test-machine.sh
#  \brief  installs basic components we need
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \note   This script may be standalone, so we do not depend it on scriptlib.sh

SCRIPTDIR=$(dirname $0)
[ -z "$CHECKOUTDIR" ] && CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
[ -z "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
export CHECKOUTDIR BUILDSUBDIR

[ -z "$MKBD_DEB" ] && MKBD_DEB="core-build-deps_0.1_all.deb"

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

limit_packages_forceremove() {
    echo "INFO: ...and just to be sure - remove some space-hungry beasts..."
    apt-get -f -y --force-yes remove --purge \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        docutils-doc libssl-doc python-docutils \
        texlive-fonts-recommended-doc texlive-latex-base-doc texlive-latex-extra-doc \
        texlive-latex-recommended-doc texlive-pictures-doc texlive-pstricks-doc
}

http_get() {
    ( which curl >/dev/null 2>&1 && \
      curl "$1" ) || \
    ( which wget >/dev/null 2>&1 && \
      wget -q -O - "$1" )
}

update_pkg_keys() {
    echo "INFO: Updating our OBS packaging keys..."
    # TODO: check if OS is debian... though this applies to all the APT magic
    http_get http://obs.roz53.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -
    # http_get http://obs.mbt.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add -

    echo "INFO: Updating upstream-distro packaging keys..."
    apt-get -f -y --force-yes \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install \
        debian-keyring debian-archive-keyring
    apt-key update
}

update_pkg_metadata() {
    echo "INFO: Refreshing packaging lists and metadata..."
    apt-get clean all
    apt-get update || { echo "Wipe metadata and retry"; rm -rf /var/lib/apt/lists/*; apt-get update; }
    dpkg --configure -a
}

install_packages_missing() {
    echo "INFO: Fixing the pre-installed set if any packages are missing..."
    apt-get -f -y --force-yes --fix-missing \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install
}

install_package_set_dev() {
    echo "INFO: Installing the predefined dev-package set..."
    apt-get -f -y --force-yes \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install \
        devscripts sudo doxygen curl git python-mysqldb \
        cppcheck msmtp libtool cpp gcc autoconf automake m4 pkg-config equivs dh-make \
	cracklib-runtime
}

install_package_set_biosdeps() {
    if [ -n "$CHECKOUTDIR" ] && [ -d "$CHECKOUTDIR" ] && [ -s "$CHECKOUTDIR/obs/core.dsc" ]; then
        echo "INFO: mk-build-deps: Installing dependencies for \$BIOS according to $CHECKOUTDIR/obs/core.dsc"
        ( cd "$CHECKOUTDIR" && mk-build-deps "./obs/core.dsc" && \
          dpkg -i "$MKBD_DEB" && \
          apt-get -f -y --force-yes \
                -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
                install )
    else
        echo "SKIPPED: mk-build-deps (CHECKOUTDIR not currently available) - will be done by autogen.sh" >&2
    fi
}

restore_ssh_service() {
    systemctl stop ssh.socket
    systemctl mask ssh.socket
    systemctl unmask ssh.service
    systemctl start ssh.service

# Note: we can fail to start the SSH service e.g. via chroot,
# so to avoid an error exit-code just report if the needed
# systemd file(s) exist
    systemctl status ssh.service || \
    [ -e /etc/systemd/system/ssh.service ] || \
    [ -e /etc/systemd/system/ssh.socket ] || \
    [ -e /etc/systemd/system/sshd.service ] || \
    [ -e /etc/systemd/system/ssh@.service ] || \
    [ -e /etc/systemd/system/sshd@.service ]
}

update_system() {
    update_pkg_keys
    update_pkg_metadata
    limit_packages_recommends
    limit_packages_paths
    install_packages_missing
    limit_packages_docs
    install_package_set_dev
    install_package_set_biosdeps
    limit_packages_forceremove
    restore_ssh_service
}

export DEBIAN_FRONTEND=noninteractive
update_system
