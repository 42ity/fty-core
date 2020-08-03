# Include file for (bash) shell profile on Linux 42ITy developer workstations
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
#! \file   ci-reset-virtual-machine.profile
#  \brief  Include file for shell profile for 42ITy devs on Linux
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details These are profile-embeddable routines that 42ITy developers
#           can add to their Linux environment to more easily manage
#           containers made from product OS images with virsh and
#           manipulated by ci-reset-virtual-machine.sh; tested with
#           bash interpreter (source this into ~root/.bashrc).
#
# ASSUMPTIONS:
#
# * containers are managed by the root account
# * there is a copy of, or symlink to, scripts from this repository
#   as /root/ci-reset-virtual-machine.sh and
#   /etc/init.d/rc-bios-ci-container-delayedstart
# * LXC manifests for containers are preconfigured, as well as the
#   storage in /srv/libvirt/... tree and optional VMNAME.saved dirs
#   for overrides of configuration for your special containers, such
#   as the IMGTYPE, IMGQALEVEL and maybe even ARCH, to set a branch.
# * If you need to manage autoupdate/autoboot of containers with
#   rc-bios-ci-container-delayedstart then you have configured your
#   /etc/default/bios-ci-container-delayedstart.conf (LOGFILE_BASENAME,
#   the list of CONTAINERS, etc.) as appropriate. It is also used in
#   the vzdownload() action below, to quickly fetch latest OS images
#   for containers of interest to a particular developer.

vzmount() {
        # $1 = VM name
        if [ -x ~root/ci-reset-virtual-machine.sh ] ; then
            echo "MOUNTING VM ROOT (without cleanup) via ~root/ci-reset-virtual-machine.sh ..." >&2
            ~root/ci-reset-virtual-machine.sh --no-download -m "$1" update mount
        fi
}

probe_mounts() {
        # $1 = VM name
        virsh dumpxml "$1" | \
        grep '<source dir=' | sed "s|^.*<source dir='\(/[^\']*\)'[ /].*>.*$|\1|" | \
        (RES=0 ; while read D ; do ls "$D/" > /dev/null || RES=$? ; done; exit $RES)
        vzmount "$1"
}

vzconsole() {
    while date; do
        echo "CONNECTING TO CONSOLE of '$1' (press Ctrl+] to close)..." >&2
        virsh console "$1"
        echo "CONSOLE of '$1' is CLOSED" >&2
        sleep 5
    done
}

vzlist() {
    virsh list --all
}

vzboot() {
    # $1 = VM name
    probe_mounts "$1"
    virsh start "$1" || \
    virsh start "$1" || \
    virsh start "$1" || \
    virsh start "$1"
}

vzhalt() {
    # $1 = VM name
    virsh destroy "$1"
}

vzreboot() {
    vzhalt "$@"
    vzboot "$@"
}

vzreboot() {
    vzhalt "$@"
    vzboot "$@"
}

vzdownload() (
    /etc/init.d/rc-bios-ci-container-delayedstart download
)

vzupgrade() (
    cd ~root && ./ci-reset-virtual-machine.sh -m "$@"
)

vzredeploy() (
    cd ~root && ./ci-reset-virtual-machine.sh --no-download --no-install-dev -m "$@"
)

vzupgrade-many() {
    while [ $# -gt 0 ]; do
        vzupgrade "$1" || return $?
        shift
    done
    return 0
}
