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
# Author(s): Michal Hrusecky <MichalHrusecky@eaton.com>,
#            Tomas Halman <TomasHalman@eaton.com>
#
# Description: Destroys VM latest, deploy new one and starts it

#
# TODO:
# * OBS parameter? [ x"$OBS_IMAGES" = x ] && OBS_IMAGES="http://obs.roz.lab.etn.com/images/"
# * should latest be a variable
# * more VM instances support?
# * set debian proxy from parameter or $http_proxy
#

die() {
	echo "$1"
	exit 1
}

usage() {
    echo "usage: $(basename $0) [options]"
    echo "options:"
    echo "    -m|--machine name    virtual machine name [latest]"
    echo "    -h|--help            print this help"
}

#
# defaults
#
VM="latest"

while [ $# -gt 0 ] ; do
    case "$1" in
        -m|--machine)
            VM="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 1
            ;;
        *)
            echo "Invalid switch $1"
            usage
            exit 1
            ;;
    esac
done

# Make sure we have a loop
modprobe loop # TODO: die on failure?

# Do we have overlayfs?
if [ "`gzip -cd /proc/config.gz 2> /dev/null | grep OVERLAY`" ]; then
	EXT="squashfs"
	OVERLAYFS="yes"
else
	EXT="tar.gz"
	OVERLAYFS=""
fi

# Get latest image for us
mkdir -p /srv/libvirt/snapshots
cd /srv/libvirt/snapshots
ARCH="`uname -m`"
IMAGE_URL="`wget -O - http://obs.roz.lab.etn.com/images/$ARCH/ 2> /dev/null | sed -n 's|.*href="\(.*simpleimage.*\.'"$EXT"'\)".*|http://obs.roz.lab.etn.com/images/'"$ARCH"'/\1|p'`"
wget -c "$IMAGE_URL"
if [ "$1" ]; then
	IMAGE="$1"
else
	IMAGE="`ls -1 *.$EXT | sort -r | head -n 1`"
fi

# Destroy whatever was running
virsh -c lxc:// destroy "$VM" 2> /dev/null > /dev/null
# may be wait for slow box
sleep 5
rm -rf "../overlays/$IMAGE"

# Mount squashfs
mkdir -p "../overlays/$IMAGE"
if [ "$OVERLAYFS" ]; then
	mkdir -p "../rootfs/$IMAGE-ro"
	umount -fl "../rootfs/$IMAGE-ro" 2> /dev/null > /dev/null
	mount -o loop "$IMAGE" "../rootfs/$IMAGE-ro" || die "Can't mount squashfs"
fi

# Cleanup of the rootfs
umount -fl "../rootfs/$VM/lib/modules" 2> /dev/null > /dev/null
umount -fl "../rootfs/$VM" 2> /dev/null > /dev/null
fusermount -u -z  "../rootfs/$VM" 2> /dev/null > /dev/null

# clean up VM space
/bin/rm -rf "../rootfs/$VM"
mkdir -p "../rootfs/$VM"
# Mount rw image
if [ "$OVERLAYFS" ]; then
	mount -t overlayfs -o lowerdir="../rootfs/$IMAGE-ro",upperdir="../overlays/$IMAGE" overlayfs "../rootfs/$VM" 2> /dev/null || die "Can't mount rw directory"
else
	mkdir -p "../rootfs/$VM"
	tar -C "../rootfs/$VM" -xzf "$IMAGE"
fi

# Bind mount modules
mkdir -p "../rootfs/$VM/lib/modules"
mount -o bind "/lib/modules ../rootfs/$VM/lib/modules"

# copy root's ~/.ssh
cp -r --preserve ~/.ssh "../rootfs/$VM/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "../rootfs/$VM/etc/ssh"

# setup debian proxy
echo 'Acquire::http::Proxy "http://gate:3142";' > "../rootfs/$VM/etc/apt/apt.conf.d/01proxy-apt-cacher"

# Start the virtual machine
virsh -c lxc:// start "$VM" || die "Can't start the virtual machine"
