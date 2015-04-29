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

# NOTE: This script is copied to VM hosts and used standalone,
# so we do not depend it on scriptlib.sh or anything else.

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-RESET-VM"
### Store some important CLI values
[ -z "$_SCRIPT_NAME" ] && _SCRIPT_NAME="$0"
_SCRIPT_ARGS="$*"
_SCRIPT_ARGC="$#"

logmsg_info() {
    echo "${LOGMSG_PREFIX}INFO: ${_SCRIPT_NAME}:" "$@"
}

logmsg_warn() {
    echo "${LOGMSG_PREFIX}WARN: ${_SCRIPT_NAME}:" "$@" >&2
}

logmsg_error() {
    echo "${LOGMSG_PREFIX}ERROR: ${_SCRIPT_NAME}:" "$@" >&2
}

die() {
    CODE="${CODE-1}"
    [ "$CODE" -ge 0 ] 2>/dev/null || CODE=1
    for LINE in "$@" ; do
        echo "${LOGMSG_PREFIX}FATAL: ${_SCRIPT_NAME}:" "$LINE" >&2
    done
    exit $CODE
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

DOTDOMAINNAME="`dnsdomainname | grep -v '('`" || \
DOTDOMAINNAME="`domainname | grep -v '('`" || \
DOTDOMAINNAME=""
[ -n "$DOTDOMAINNAME" ] && DOTDOMAINNAME=".$DOTDOMAINNAME"

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
	    logmsg_error "Invalid switch $1"
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
	logmsg_info "Detected support of OVERLAYFS on the host" \
	    "`hostname`${DOTDOMAINNAME}, so will mount a .$EXT file" \
	    "as an RO base and overlay the RW changes"
else
	EXT="tar.gz"
	OVERLAYFS=""
	logmsg_info "Detected no support of OVERLAYFS on the host" \
	    "`hostname`${DOTDOMAINNAME}, so will unpack a .$EXT file" \
	    "into a dedicated full RW directory"
fi

# Note: several hardcoded paths are expected relative to this one,
# so it is critical that we succeed changing into this directory.
mkdir -p /srv/libvirt/snapshots
cd /srv/libvirt/snapshots || \
	die "Can not 'cd /srv/libvirt/snapshots' to download image files"

logmsg_info "Get the latest operating environment image prepared for us by OBS"
ARCH="`uname -m`"
IMAGE_URL="`wget -O - $OBS_IMAGES/$IMGTYPE/$ARCH/ 2> /dev/null | sed -n 's|.*href="\(.*simpleimage.*\.'"$EXT"'\)".*|'"$OBS_IMAGES/$IMGTYPE/$ARCH"'/\1|p' | sed 's,\([^:]\)//,\1/,g'`"
wget -c "$IMAGE_URL"
if [ "$1" ]; then
	# TODO: We should not get to this point in current code structure
	# (CLI parsing loop above would fail on unrecognized parameter).
	# Should think if anything must be done about picking a particular
	# image name for a particular VM, though.
	IMAGE="$1"
else
	IMAGE="`ls -1 *.$EXT | sort -r | head -n 1`"
fi
logmsg_info "Will use IMAGE='$IMAGE' for further VM set-up"

# Destroy whatever was running
virsh -c lxc:// destroy "$VM" 2> /dev/null > /dev/null
# may be wait for slow box
sleep 5

# Destroy the overlay-rw half of the old running container, if any
if [ -d "../overlays/${IMAGE}__${VM}" ]; then
	logmsg_info "Removing RW directory of the stopped VM:" \
		"'../overlays/${IMAGE}__${VM}'"
	rm -rf "../overlays/${IMAGE}__${VM}"
	sleep 1; echo ""
fi

# When the host gets ungracefully rebooted, useless old dirs may remain...
for D in ../overlays/*__${VM}/ ; do
	if [ -d "$D" ]; then
		logmsg_warn "Obsolete RW directory for an old version" \
			"of this VM was found, removing '`pwd`/$D':"
		ls -lad "$D"
		rm -rf "$D"
		sleep 1; echo ""
	fi
done
for D in ../rootfs/*-ro/ ; do
	# Do not remove the current IMAGE mountpoint if we reuse it again now
	[ x"$D" = x"../rootfs/$IMAGE-ro/" ] && continue
	# Now, ignore non-directories and not-empty dirs (used mountpoints)
	if [ -d "$D" ] && [ x"`cd $D && find .`" = x. ]; then
		# This is a directory, and it is empty
		FD="`cd "$D" && pwd`" && \
		    [ x"`mount | grep ' on '${FD}' type '`" != x ] && \
		    logmsg_warn "Old RO mountpoint '$FD' seems still used" && \
		    continue

		logmsg_warn "Obsolete RO mountpoint for this IMAGE was found," \
		    "removing '`pwd`/$D':"
		ls -lad "$D"; ls -la "$D"
		umount -fl "$D" 2> /dev/null > /dev/null
		rm -rf "$D"
		sleep 1; echo ""
	fi
done

# Cleanup of the rootfs
logmsg_info "Unmounting paths related to VM '$VM':" \
	"'`pwd`/../rootfs/$VM/lib/modules'," \
	"'`pwd`/../rootfs/$VM', '`pwd`/../rootfs/$IMAGE-ro'"
umount -fl "../rootfs/$VM/lib/modules" 2> /dev/null > /dev/null
umount -fl "../rootfs/$VM" 2> /dev/null > /dev/null
fusermount -u -z  "../rootfs/$VM" 2> /dev/null > /dev/null

# This unmount can fail if for example several containers use the same RO image
# or if it is not used at all; not shielding by "$OVERLAYFS" check just in case
umount -fl "../rootfs/$IMAGE-ro" 2> /dev/null > /dev/null || true

# clean up VM space
logmsg_info "Removing VM rootfs from '`pwd`/../rootfs/$VM'"
/bin/rm -rf "../rootfs/$VM"

if [ x"$STOPONLY" = xyes ]; then
	logmsg_info "STOPONLY was requested, so ending" \
		"'${_SCRIPT_NAME} ${_SCRIPT_ARGS}' now" >&2
	exit 0
fi

logmsg_info "Creating a new VM rootfs at '`pwd`/../rootfs/$VM'"
mkdir -p "../rootfs/$VM"
if [ "$OVERLAYFS" = yes ]; then
	logmsg_info "Mount the common RO squashfs at '`pwd`/../rootfs/$IMAGE-ro'"
	mkdir -p "../rootfs/$IMAGE-ro"
	mount -o loop "$IMAGE" "../rootfs/$IMAGE-ro" || \
		die "Can't mount squashfs"

	logmsg_info "Use the individual RW component" \
		"located in '`pwd`/../overlays/${IMAGE}__${VM}'" \
		"for an overlay-mount united at '`pwd`/../rootfs/$VM'"
	mkdir -p "../overlays/${IMAGE}__${VM}"
	mount -t overlayfs \
	    -o lowerdir="../rootfs/$IMAGE-ro",upperdir="../overlays/${IMAGE}__${VM}" \
	    overlayfs "../rootfs/$VM" 2> /dev/null \
	|| die "Can't overlay-mount rw directory"
else
	logmsg_info "Unpack the full individual RW copy of the image" \
		"'$IMAGE' at '`pwd`/../rootfs/$VM'"
	tar -C "../rootfs/$VM" -xzf "$IMAGE" \
	|| die "Can't un-tar the rw directory"
fi

logmsg_info "Bind-mount kernel modules from the host OS"
mkdir -p "../rootfs/$VM/lib/modules"
mount -o rbind "/lib/modules" "../rootfs/$VM/lib/modules"
mount -o remount,ro,rbind "../rootfs/$VM/lib/modules"

logmsg_info "Setup virtual hostname"
echo "$VM" > "../rootfs/$VM/etc/hostname"
logmsg_info "Put virtual hostname in resolv.conf"
sed -r -i "s/^127\.0\.0\.1/127.0.0.1 $VM /" "../rootfs/$VM/etc/hosts"

logmsg_info "Copy root's ~/.ssh from the host OS"
cp -r --preserve ~/.ssh "../rootfs/$VM/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "../rootfs/$VM/etc/ssh"

logmsg_info "Copy environment settings from the host OS"
cp /etc/profile.d/* ../rootfs/$VM/etc/profile.d/

logmsg_info "Add xterm terminfo from the host OS"
mkdir -p ../rootfs/$VM/lib/terminfo/x
cp /lib/terminfo/x/xterm* ../rootfs/$VM/lib/terminfo/x

mkdir -p "../rootfs/$VM/etc/apt/apt.conf.d/"
# setup debian proxy
[ -n "$APT_PROXY" ] && \
	logmsg_info "Set up APT configuration" && \
	echo 'Acquire::http::Proxy "'"$APT_PROXY"'";' > \
		"../rootfs/$VM/etc/apt/apt.conf.d/01proxy-apt-cacher"

logmsg_info "Start the virtual machine"
virsh -c lxc:// start "$VM" || die "Can't start the virtual machine"

logmsg_info "Preparation and startup of the virtual machine '$VM'" \
	"is successfully completed on `date -u` on host" \
	"'`hostname`${DOTDOMAINNAME}'"

exit 0
