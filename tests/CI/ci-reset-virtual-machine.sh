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
#            Tomas Halman <TomasHalman@eaton.com>,
#            Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: Destroys VM "latest" (or named by -m), deploys a new
# one from image prepared by OBS, and starts it

#
# TODO:
# * more VM instances support?
# ** (multiple VMs in one call?)
# * set debian proxy from parameter or $http_proxy
#

# NOTE: This script may be standalone, so we do not depend it on scriptlib.sh
die() {
	echo "$1" >&2
	exit 1
}

usage() {
    echo "Usage: $(basename $0) [options...]"
    echo "options:"
    echo "    -m|--machine name    virtual machine libvirt-name (Default: '$VM')"
    echo "    -b|--baseline type   basic image type to use (Default: '$IMGTYPE')"
    echo "                         see OBS repository for supported types (deploy, devel)"
    echo "    -r|--repository URL  OBS image repo ('$OBS_IMAGES')"
    echo "    -hp|--http-proxy URL the http_proxy override to access OBS ('$http_proxy')"
    echo "    -ap|--apt-proxy URL  the http_proxy to access external APT images ('$APT_PROXY')"
    echo "    --stop-only          end the script after stopping the VM and cleaning up"
    echo "    -h|--help            print this help"
}

#
# defaults
#
VM="latest"
[ -z "$IMGTYPE" ] && IMGTYPE="deploy"
[ -z "$OBS_IMAGES" ] && OBS_IMAGES="http://obs.roz.lab.etn.com/images/"
[ -z "$APT_PROXY" ] && APT_PROXY='http://gate.roz.lab.etn.com:3142'
[ -n "$http_proxy" ] && export http_proxy

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

[ -z "$STOPONLY" ] && STOPONLY=no

while [ $# -gt 0 ] ; do
    case "$1" in
	-m|--machine)
	    VM="$2"
	    shift 2
	    ;;
	-b|--baseline)
	    IMGTYPE="$2"
	    shift 2
	    ;;
	-r|--repository)
	    OBS_IMAGES="$2"
	    shift 2
	    ;;
	-hp|--http-proxy)
	    http_proxy="$2"
	    export http_proxy
	    shift 2
	    ;;
	-ap|--apt-proxy)
	    [ -z "$2" ] && APT_PROXY="-" || APT_PROXY="$2"
	    shift 2
	    ;;
	--stop-only)
	    STOPONLY=yes
	    shift
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

#
# check if VM exists
#
RESULT=$(virsh -c lxc:// list --all | awk '/^ *[0-9-]+ +'"$VM"' / {print $2}' | wc -l)
if [ $RESULT = 0 ] ; then
    die "VM $VM does not exist"
fi
if [ $RESULT -gt 1 ] ; then
    ### Should not get here via CI
    die "VM pattern '$VM' matches too much ($RESULT)"
fi

# This should not be hit...
[ -z "$APT_PROXY" ] && APT_PROXY="$http_proxy"
[ x"$APT_PROXY" = x- ] && APT_PROXY=""
[ x"$http_proxy" = x- ] && http_proxy="" && export http_proxy

# Make sure we have a loop device support
modprobe loop # TODO: die on failure?

# Do we have overlayfs in kernel?
if \
	[ "`gzip -cd /proc/config.gz 2>/dev/null | grep OVERLAY`" ] || \
	grep OVERLAY "/boot/config-`uname -r`" >/dev/null 2>/dev/null  \
; then
	EXT="squashfs"
	OVERLAYFS="yes"
	echo "Detected support of OVERLAYFS on the `hostname` host," \
	    "will mount a .$EXT file as an RO base and overlay the RW changes"
else
	EXT="tar.gz"
	OVERLAYFS=""
	echo "Detected no support of OVERLAYFS on the `hostname` host," \
	    "will unpack a .$EXT file into a dedicated full RW directory"
fi

# Note: several hardcoded paths are expected relative to this one,
# so it is critical that we succeed changing into this directory.
mkdir -p /srv/libvirt/snapshots
cd /srv/libvirt/snapshots || \
	die "FATAL: Can not 'cd /srv/libvirt/snapshots' to download image files"

# Get latest image for us
ARCH="`uname -m`"
IMAGE_URL="`wget -O - $OBS_IMAGES/$IMGTYPE/$ARCH/ 2> /dev/null | sed -n 's|.*href="\(.*simpleimage.*\.'"$EXT"'\)".*|'"$OBS_IMAGES/$IMGTYPE/$ARCH"'/\1|p' | sed 's,\([^:]\)//,\1/,g'`"
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

# Destroy the overlay-rw half of the old running container, if any
[ -d "../overlays/${IMAGE}__${VM}" ] && \
	echo "INFO: Removing RW directory of the stopped VM:" \
		"'../overlays/${IMAGE}__${VM}'" && \
	rm -rf "../overlays/${IMAGE}__${VM}"

# When the host gets ungracefully rebooted, useless old dirs may remain...
for D in ../overlays/*__${VM}/ ; do
	[ -d "$D" ] && echo "WARN: Obsolete RW directory for this VM was found," \
		"removing '`pwd`/$D'..." >&2 && \
		{ ls -lad "$D"; rm -rf "$D"; }
done
for D in ../rootfs/*-ro/ ; do
	# Do not touch the current IMAGE mountpoint if we reuse it
	[ x"$D" = x"../rootfs/$IMAGE-ro/" ] && continue
	if [ -d "$D" ] && [ x"`cd $D && find .`" = x ]; then
		# This is a directory, and it is empty
		FD="`cd "$D" && pwd`" && \
		    [ x"`mount | grep ' on '${FD}' type '`" != x ] && \
		    echo "INFO: Old RO mountpoint '$D' seems still used" >&2 && \
		    continue

		echo "WARN: Obsolete RO mountpoint for this IMAGE was found," \
		    "removing '`pwd`/$D'..." >&2
		ls -la "$D"
		umount -fl "$D" 2> /dev/null > /dev/null
		rm -rf "$D"
	fi
done

# Cleanup of the rootfs
umount -fl "../rootfs/$VM/lib/modules" 2> /dev/null > /dev/null
umount -fl "../rootfs/$VM" 2> /dev/null > /dev/null
fusermount -u -z  "../rootfs/$VM" 2> /dev/null > /dev/null

umount -fl "../rootfs/$IMAGE-ro" 2> /dev/null > /dev/null

# clean up VM space
/bin/rm -rf "../rootfs/$VM"

if [ x"$STOPONLY" = xyes ]; then
	echo "INFO: STOPONLY was requested, so ending '$0 $@' now" >&2
	exit 0
fi

mkdir -p "../rootfs/$VM"
# Mount RO squashfs
if [ "$OVERLAYFS" = yes ]; then
	mkdir -p "../overlays/${IMAGE}__${VM}"
	mkdir -p "../rootfs/$IMAGE-ro"
	mount -o loop "$IMAGE" "../rootfs/$IMAGE-ro" || \
		die "Can't mount squashfs"
fi

# Mount RW image
if [ "$OVERLAYFS" = yes ]; then
	mount -t overlayfs \
	    -o lowerdir="../rootfs/$IMAGE-ro",upperdir="../overlays/${IMAGE}__${VM}" \
	    overlayfs "../rootfs/$VM" 2> /dev/null \
	|| die "Can't overlay-mount rw directory"
else
	tar -C "../rootfs/$VM" -xzf "$IMAGE" \
	|| die "Can't un-tar the rw directory"
fi

# Bind-mount kernel modules from the host OS
mkdir -p "../rootfs/$VM/lib/modules"
mount -o rbind "/lib/modules" "../rootfs/$VM/lib/modules"
mount -o remount,ro,rbind "../rootfs/$VM/lib/modules"

# copy root's ~/.ssh from the host OS
cp -r --preserve ~/.ssh "../rootfs/$VM/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "../rootfs/$VM/etc/ssh"

mkdir -p "../rootfs/$VM/etc/apt/apt.conf.d/"
# setup debian proxy
[ -n "$APT_PROXY" ] && \
	echo 'Acquire::http::Proxy "'"$APT_PROXY"'";' > \
		"../rootfs/$VM/etc/apt/apt.conf.d/01proxy-apt-cacher"
#echo 'APT::Install-Recommends "false";' > \
#	"../rootfs/$VM/etc/apt/apt.conf.d/02no-recommends"

# setup virtual hostname
echo "$VM" > "../rootfs/$VM/etc/hostname"

# add xterm terminfo from the host OS
mkdir -p ../rootfs/$VM/lib/terminfo/x
cp /lib/terminfo/x/xterm* ../rootfs/$VM/lib/terminfo/x

# copy enviroment settings from the host OS
cp /etc/profile.d/* ../rootfs/$VM/etc/profile.d/

# put hostname in resolv.conf
sed -r -i "s/^127\.0\.0\.1/127.0.0.1 $VM /" "../rootfs/$VM/etc/hosts"

# try to influence dpkg
# see also "dpkg --set-selections" in ci-setup-test-machine.sh
mkdir -p ../rootfs/$VM/etc/dpkg/dpkg.cfg.d
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
' >> ../rootfs/$VM/etc/dpkg/dpkg.cfg.d/excludes

# Start the virtual machine
virsh -c lxc:// start "$VM" || die "Can't start the virtual machine"
