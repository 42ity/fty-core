#!/bin/bash
#
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
#! \file   ci-setup-test-machine.sh
#  \brief  installs basic components we need
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \note   This script may be standalone, so we do not depend it on scriptlib.sh

set -o pipefail

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

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-SETUPVM-"
### Store some important CLI values
[ -z "$_SCRIPT_PATH" ] && _SCRIPT_PATH="$0"
[ -z "$_SCRIPT_NAME" ] && _SCRIPT_NAME="`basename "${_SCRIPT_PATH}"`"
_SCRIPT_ARGS="$*"
_SCRIPT_ARGC="$#"

# NOTE: This script may be standalone, so we do not depend it on scriptlib.sh
SCRIPTDIR=$(realpath `dirname ${_SCRIPT_PATH}`)
SCRIPTPWD="`pwd`"
[ -z "$CHECKOUTDIR" ] && CHECKOUTDIR=$(realpath $SCRIPTDIR/../..)
[ "$CHECKOUTDIR" = / -o ! -d "$CHECKOUTDIR/tests/CI" ] && CHECKOUTDIR=""
[ -z "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
export CHECKOUTDIR BUILDSUBDIR

logmsg_info() {
        echo "${LOGMSG_PREFIX}INFO: ${_SCRIPT_PATH}:" "$@"
}

logmsg_warn() {
        echo "${LOGMSG_PREFIX}WARN: ${_SCRIPT_PATH}:" "$@" >&2
}

logmsg_error() {
        echo "${LOGMSG_PREFIX}ERROR: ${_SCRIPT_PATH}:" "$@" >&2
}

die() {
        CODE="${CODE-1}"
        [ "$CODE" -ge 0 ] 2>/dev/null || CODE=1
        for LINE in "$@" ; do
                echo "${LOGMSG_PREFIX}FATAL: ${_SCRIPT_PATH}:" "$LINE" >&2
        done
        exit $CODE
}

# Match CPU arch to packaging arch
[ -z "$ARCH" ] && ARCH="`uname -m`"
[ -z "$ARCH_PKG" ] && case "$ARCH" in
    x86_64|amd64) ARCH_PKG="amd64" ;;
    armv7l|armhf) ARCH_PKG="armhf";;
esac

limit_packages_recommends() {
    echo "INFO: Tell APT to not install packages from 'Recommends' category"
    mkdir -p /etc/apt/apt.conf.d
    echo 'APT::Install-Recommends "false";' > \
       "/etc/apt/apt.conf.d/02no-recommends"
}

limit_packages_expiration_check() {
    echo "INFO: Tell APT to not ignore repositories that were not updated recently"
    mkdir -p /etc/apt/apt.conf.d
    echo 'Acquire::Check-Valid-Until "false";' > \
       "/etc/apt/apt.conf.d/02no-expiration"
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
    ( RES=0
      for P in \
        docutils-doc libssl-doc python-docutils \
        texlive-fonts-recommended-doc \
        texlive-latex-base-doc \
        texlive-latex-extra-doc \
        texlive-latex-recommended-doc \
        texlive-pictures-doc \
        texlive-pstricks-doc \
      ; do
        apt-mark hold "$P" >&2 # || RES=$?
        echo "$P  purge"
      done ; exit $RES ) | dpkg --set-selections
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
    local RES=0
    echo "INFO: Updating our OBS packaging keys..."
    # TODO: check if OS is debian... though this applies to all the APT magic
    http_get http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add - || RES=$?
    # http_get http://obs.roz53.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add - || RES=$?
    # http_get http://obs.mbt.lab.etn.com:82/Pool:/master/Debian_8.0/Release.key | apt-key add - || RES=$?

    echo "INFO: Updating upstream-distro packaging keys..."
    apt-get -f -y --force-yes \
        -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
        install \
        debian-keyring debian-archive-keyring || RES=$?
    apt-key update || RES=$?
    return $RES
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
        devscripts sudo doxygen curl git python-mysqldb libmariadbclient-dev \
        cppcheck msmtp libtool cpp gcc autoconf automake m4 pkg-config equivs dh-make zip \
        cracklib-runtime osc
}

install_package_set_biosdeps() {
    if [ -n "$CHECKOUTDIR" ] && [ -d "$CHECKOUTDIR" ] && [ -s "$CHECKOUTDIR/obs/core.dsc" ]; then
        echo "INFO: mk-build-deps: Installing dependencies for 42ity according to $CHECKOUTDIR/obs/core.dsc"
        ( cd "$CHECKOUTDIR" && mk-build-deps "./obs/core.dsc" && \
          dpkg -i "$MKBD_DEB" && \
          apt-get -f -y --force-yes \
                -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
                install )
    else
        echo "SKIPPED: mk-build-deps (CHECKOUTDIR not currently available) - will be done by autogen.sh" >&2
    fi
}

install_package_set_java8_jre() {
    # Needed for Jenkins workers, Flexnet CLI tools, ...
    # Magic variable can be set and exported by caller
    if [ -n "${DEPLOY_JAVA8-}" ] && [ "${DEPLOY_JAVA8-}" != no ]; then
        echo "INFO: Installing the predefined package set for Java 8 JRE..."
        yes | apt-get -y \
            -q -o Dpkg::Options::="--force-confdef" -o Dpkg::Options::="--force-confold" \
            install -t jessie-backports \
            openjdk-8-jre-headless && \
        update-java-alternatives --set java-1.8.0-openjdk \
        || update-java-alternatives --set java-1.8.0-openjdk-${ARCH_PKG} \
        || ln -s /usr/lib/jvm/java-8-openjdk-${ARCH_PKG}/bin/java /usr/bin/java
    fi
}

install_package_set_libzmq4_dev() {
    # Needed for builds against specifically upstream libzmq, not our default fork
    # Magic variable can be set and exported by caller
    if [ -n "${DEPLOY_LIBZMQ4_DEV-}" ] && [ "${DEPLOY_LIBZMQ4_DEV-}" != no ]; then
        echo "INFO: Installing the upstream libzmq4-dev, not our default fork..."
        yes | apt-get install -y --force-yes libzmq4-dev
    fi
}

restore_ssh_service() {
    /bin/systemctl stop ssh.socket
    /bin/systemctl mask ssh.socket
    /bin/systemctl unmask ssh.service
    /bin/systemctl start ssh.service

# Note: we can fail to start the SSH service e.g. via chroot,
# so to avoid an error exit-code just report if the needed
# systemd file(s) exist
    /bin/systemctl status ssh.service || \
    [ -e /etc/systemd/system/ssh.service ] || \
    [ -e /etc/systemd/system/ssh.socket ] || \
    [ -e /etc/systemd/system/sshd.service ] || \
    [ -e /etc/systemd/system/ssh@.service ] || \
    [ -e /etc/systemd/system/sshd@.service ]
}

update_system() {
    if [[ -n "${FORCE_RUN_APT}" ]]; then
        # Die on failures, so callers know that VM setup did not go as planned
        limit_packages_expiration_check || die "Failed to limit_packages_expiration_check()"
        update_pkg_keys || die "Failed to update_pkg_keys()"
        update_pkg_metadata || die "Failed to update_pkg_metadata()"
        limit_packages_recommends || die "Failed to limit_packages_recommends()"
        limit_packages_paths || die "Failed to limit_packages_paths()"
        install_packages_missing || install_packages_missing || die "Failed to install_packages_missing()"
        limit_packages_docs || die "Failed to limit_packages_docs()"
        install_package_set_dev || install_package_set_dev || die "Failed to install_package_set_dev()"
        install_package_set_biosdeps || install_package_set_biosdeps || die "Failed to install_package_set_biosdeps()"
        install_package_set_java8_jre || install_package_set_java8_jre || die "Failed to install_package_set_java8_jre()"
        install_package_set_libzmq4_dev || install_package_set_libzmq4_dev || die "Failed to install_package_set_libzmq4_dev()"
        limit_packages_forceremove || die "Failed to limit_packages_forceremove()"
    else
        echo "SKIPPED: $0 update_system() : this action is not default anymore, and FORCE_RUN_APT is not set and exported by caller" >&2
    fi

    # Here an exit-code suffices
    set +e
    restore_ssh_service
}

export DEBIAN_FRONTEND=noninteractive
update_system
