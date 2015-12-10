#!/bin/sh
#
# Copyright (C) 2014-2015 Eaton
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
#! \file   ci-reset-virtual-machine.sh
#  \brief  Destroys VM "latest" (or one named by -m), deploys a new
#  \brief  one from image prepared by OBS, and starts it
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \todo   more VM instances support? (multiple VMs in one call?)
#          set debian proxy from parameter or $http_proxy
#

# NOTE: This script is copied to VM hosts and used standalone,
# so we do not depend it on scriptlib.sh or anything else.

### This is prefixed before ERROR, WARN, INFO tags in the logged messages
[ -z "$LOGMSG_PREFIX" ] && LOGMSG_PREFIX="CI-RESETVM-"
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

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

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

usage() {
    echo "Usage: ${_SCRIPT_NAME} [options...]"
    echo "options:"
    echo "    -m|--machine name    virtual machine libvirt-name (Default: '$VM')"
    echo "    -b|--baseline type   basic image type to use (Default: '$IMGTYPE')"
    echo "                         see OBS repository for supported types (deploy, devel)"
    echo "    -r|--repository URL  OBS image repo ('$OBS_IMAGES')"
    echo "    -hp|--http-proxy URL the http_proxy override to access OBS ('$http_proxy')"
    echo "    -ap|--apt-proxy URL  the http_proxy to access external APT images ('$APT_PROXY')"
    echo "    --install-dev        run ci-setup-test-machine.sh (if available) to install packages"
    echo "    --download-only      end the script after downloading the newest image file"
    echo "    --attempt-download [auto|yes|no] Should an OS image download be attempted at all?"
    echo "                         (default: auto; default if only the option is specified: yes)"
    echo "    --stop-only          end the script after stopping the VM and cleaning up"
    echo "    --deploy-only        end the script just before it would start the VM (skips apt-get too)"
    echo "    --copy-host-users 'a b c'    Copies specified user or group account definitions"
    echo "    --copy-host-groups 'a b c'   (e.g. for bind-mounted homes from host into the VM)"
    echo "    --no-config-file     Forbid use in this run of a per-VM config file if one is found"
    echo "    -h|--help            print this help"
}

check_md5sum() {
	# Compares actual checksum of file "$1" with value recorded in file "$2"
	if [ -s "$1" -a -s "$2" ]; then
		logmsg_info "Validating OS image file '$1' against its MD5 checksum '$2'..."
		MD5EXP="`awk '{print $1}' < "$2"`"
		MD5ACT="`md5sum < "$1" | awk '{print $1}'`" && \
		if [ x"$MD5EXP" != x"$MD5ACT" ]; then
			logmsg_error "Checksum validation of '$1' against '$2' FAILED!"
			return 1
		fi
		logmsg_info "Checksum validation of '$1' against '$2' SUCCEEDED!"
		return 0
	fi
	logmsg_warn "Checksum validation of '$1' against '$2' SKIPPED (one of the files is missing)"
	return 0
}

ensure_md5sum() {
	# A destructive wrapper of check_md5sum(), destroys bad downloads
	if ! check_md5sum "$@" ; then
		logmsg_warn "Removing broken file: '$IMAGE'"
		rm -f "$IMAGE" "$IMAGE.md5"
		return 1
	fi
	return 0
}

cleanup_script() {
	# Note that by default we currently have VM=latest
	[ -z "$VM" ] && return 0
	if [ -f "/srv/libvirt/rootfs/$VM.lock" ] ; then
		rm -f "/srv/libvirt/rootfs/$VM.lock"
	fi

	[ "$ERRCODE" -ge 0 ] 2>/dev/null && \
	for F in `ls -1 /srv/libvirt/rootfs/$VM.wantexitcode.* 2>/dev/null || true` ; do
		echo "$ERRCODE" > "$F"
	done
}

cleanup_wget() {
	[ -z "$IMAGE" ] && return 0
	[ "$WGET_RES" != 0 ] && rm -f "$IMAGE" "$IMAGE.md5"
	rm -f "$IMAGE.lock"
}

settraps() {
	# Not all trap names are recognized by all shells consistently
	# Note: slight difference from scriptlib.sh, we trap ERR too by default
	[ -z "${TRAP_SIGNALS-}" ] && TRAP_SIGNALS="ERR  EXIT QUIT TERM HUP INT"
	for P in "" SIG; do for S in $TRAP_SIGNALS ; do
		case "$1" in
		-|"") trap "$1" $P$S 2>/dev/null || true ;;
		*)    trap 'ERRCODE=$?; ('"$*"'); exit $ERRCODE;' $P$S 2>/dev/null || true ;;
		esac
	done; done
}

#
# defaults
#
### TODO: Assign this default later in the script, after downloads
VM="latest"
[ -z "$IMGTYPE" ] && IMGTYPE="devel"
[ -z "$OBS_IMAGES" ] && OBS_IMAGES="http://obs.roz53.lab.etn.com/images/"
[ -z "$APT_PROXY" ] && APT_PROXY='http://gate.roz53.lab.etn.com:3142'
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
[ -z "$DOWNLOADONLY" ] && DOWNLOADONLY=no
[ -z "$DEPLOYONLY" ] && DEPLOYONLY=no
[ -z "$INSTALL_DEV_PKGS" ] && INSTALL_DEV_PKGS=no
[ -z "$ATTEMPT_DOWNLOAD" ] && ATTEMPT_DOWNLOAD=auto
[ -z "$ALLOW_CONFIG_FILE" ] && ALLOW_CONFIG_FILE=yes

while [ $# -gt 0 ] ; do
    case "$1" in
	-m|--machine)
	    VM="`echo "$2" | tr '/' '\!'`"
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
	--download-only)
	    DOWNLOADONLY=yes
	    shift
	    ;;
	--deploy-only)
	    DEPLOYONLY=yes
	    shift
	    ;;
	--attempt-download)
	    shift
	    case "$1" in
		yes|no|auto) ATTEMPT_DOWNLOAD="$1"; shift ;;
		*) ATTEMPT_DOWNLOAD=yes ;;
	    esac
	    ;;
	--install-dev|--install-dev-pkgs)
	    INSTALL_DEV_PKGS=yes
	    shift
	    ;;
	--copy-host-users)
	    COPYHOST_USERS="$2"; shift 2;;
	--copy-host-groups)
	    COPYHOST_GROUPS="$2"; shift 2;;
	--no-config-file)
	    ALLOW_CONFIG_FILE=no; shift ;;
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
### TODO: Actions dependent on a particular "$VM" begin after image downloads
[ -z "$VM" ] && die "VM parameter not provided!"
RESULT=$(virsh -c lxc:// list --all | awk '/^ *[0-9-]+ +'"$VM"' / {print $2}' | wc -l)
if [ $RESULT = 0 ] ; then
    die "VM $VM does not exist"
fi
if [ $RESULT -gt 1 ] ; then
    ### Should not get here via CI
    die "VM pattern '$VM' matches too much ($RESULT)"
    ### TODO: spawn many child-shells with same parameters, for each VM?
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
	modprobe squashfs
	for OVERLAYFS_TYPE in overlay overlayfs ; do
		modprobe ${OVERLAYFS_TYPE} && break
	done
else
	EXT="tar.gz"
	OVERLAYFS=""
	OVERLAYFS_TYPE=""
	logmsg_info "Detected no support of OVERLAYFS on the host" \
	    "`hostname`${DOTDOMAINNAME}, so will unpack a .$EXT file" \
	    "into a dedicated full RW directory"
fi

[ -z "$ARCH" ] && ARCH="`uname -m`"
# Note: several hardcoded paths are expected relative to "snapshots", so
# it is critical that we succeed changing into this directory in the end.
mkdir -p "/srv/libvirt/snapshots/$IMGTYPE/$ARCH" "/srv/libvirt/rootfs" "/srv/libvirt/overlays"
cd "/srv/libvirt/rootfs" || \
	die "Can not 'cd /srv/libvirt/rootfs' to keep container root trees"

if [ -s "$VM.config-reset-vm" ]; then
	if [ "$ALLOW_CONFIG_FILE" = yes ]; then
		logmsg_warn "Found configuration file for the '$VM', it will override the command-line settings!"
		. "$VM.config-reset-vm" || die "Can not import config file '`pwd`/$VM.config-reset-vm'"
	else
		logmsg_warn "Found configuration file for the '$VM', but it is ignored because ALLOW_CONFIG_FILE='$ALLOW_CONFIG_FILE'"
	fi
fi

# Verify that this script runs once at a time for the given VM
if [ -f "$VM.lock" ] ; then
	OTHERINST_PID="`head -1 "$VM.lock"`"
	OTHERINST_PROG="`head -n +2 "$VM.lock" | tail -1`"
	OTHERINST_ARGS="`head -n +3 "$VM.lock" | tail -1`"
	if [ -n "$OTHERINST_PID" ] && \
	   [ "$OTHERINST_PID" -gt 0 ]  2>/dev/null  && \
	   [ -d "/proc/$OTHERINST_PID" ] && \
	   ps -ef | awk '( $2 == "'"$OTHERINST_PID"'") {print $0}' | egrep "${_SCRIPT_NAME}|sh " \
	; then
		if [ x"$OTHERINST_ARGS" = x"${_SCRIPT_ARGS}" ]; then
			logmsg_info "`date`: An instance of this script with PID $OTHERINST_PID and the same parameters is already running," \
				"now waiting for it to finish"
			# Catch exit-code of that invokation, and return if ok
			# or even better, retry if that exit was a failure
			settraps "rm -f $VM.wantexitcode.$$;"
			touch "$VM.wantexitcode.$$"
			while [ -f "$VM.lock" ] && [ -d "/proc/$OTHERINST_PID" ]; do sleep 1; done
			OTHERINST_EXIT="`cat "$VM.wantexitcode.$$"`" || OTHERINST_EXIT=""
			[ -n "$OTHERINST_EXIT" ] && [ "$OTHERINST_EXIT" -ge 0 ] 2>/dev/null || OTHERINST_EXIT=""
			rm -f "$VM.wantexitcode.$$"
			settraps -
			if [ "$OTHERINST_EXIT" = 0 ]; then
				logmsg_info "`date`: Wait is complete, finishing script as the requested job is done OK"
				exit 0
			fi
			logmsg_info "`date`: Wait is complete, the other instance returned code '$OTHERINST_EXIT' so retrying"
		else
			# TODO: Flag to choose whether to wait-and-continue
			# or to abort (with error?) at this point
			logmsg_info "`date`: An instance of this script with PID $OTHERINST_PID and different parameters ($OTHERINST_ARGS) is already running," \
				"now waiting for it to finish and will continue with my requested task (${_SCRIPT_ARGS})"
			while [ -f "$VM.lock" ] && [ -d "/proc/$OTHERINST_PID" ]; do sleep 1; done
			logmsg_info "`date`: Wait is complete, continuing with my task (${_SCRIPT_ARGS})"
		fi
	else
		logmsg_info "Found lock-file `pwd`/$VM.lock, but it is invalid or not up-to-date (ignoring)"
	fi
fi

( echo "$$"; echo "${_SCRIPT_PATH}"; echo "${_SCRIPT_ARGS}" ) > "$VM.lock"
settraps 'cleanup_script'

# Proceed to downloads, etc.
cd "/srv/libvirt/snapshots/$IMGTYPE/$ARCH" || \
	die "Can not 'cd /srv/libvirt/snapshots/$IMGTYPE/$ARCH' to download image files"

# Initial value, aka "file not found"
WGET_RES=127
IMAGE=""
IMAGE_SKIP=""

if [ "$ATTEMPT_DOWNLOAD" != no ] ; then
	logmsg_info "Get the latest operating environment image prepared for us by OBS"
	IMAGE_URL="`wget -O - $OBS_IMAGES/$IMGTYPE/$ARCH/ 2> /dev/null | sed -n 's|.*href="\(.*simpleimage.*\.'"$EXT"'\)".*|'"$OBS_IMAGES/$IMGTYPE/$ARCH"'/\1|p' | sed 's,\([^:]\)//,\1/,g'`"
	IMAGE="`basename "$IMAGE_URL"`"

	# Set up sleeping
	MAXSLEEP=240
	SLEEPONCE=5
	NUM="`expr $MAXSLEEP / $SLEEPONCE`"

	while [ -f "$IMAGE.lock" ] && [ "$NUM" -gt 0 ]; do
		if WGETTER_PID="`cat "$IMAGE.lock"`" ; then
			# TODO: This locking method is only good on a local system,
			# not on shared networked storage where fuser, flock() or
			# tracking metadata changes over time perform more reliably.
			if [ -n "$WGETTER_PID" ] && [ "$WGETTER_PID" -gt 0 ] 2>/dev/null && [ -d "/proc/$WGETTER_PID" ] ; then
				ps -ef | \
				awk '( $2 == "'"$WGETTER_PID"'") {print $0}' | egrep "${_SCRIPT_NAME}|sh " \
					|| WGETTER_PID=""
			else
				WGETTER_PID=""
			fi

			if [ -z "$WGETTER_PID" ] ; then
				logmsg_info "Lock-file at '$IMAGE.lock' seems out of date, skipping"
				#NUM=-1
				rm -f "$IMAGE.lock"
				sleep $SLEEPONCE
				continue	# Maybe another sleeper grabs the lock
			fi
		fi
		[ "`expr $NUM % 3`" = 0 ] && \
			logmsg_warn "Locked out of downloading '$IMAGE' by PID '$WGETTER_PID'," \
				"waiting for `expr $NUM \* $SLEEPONCE` more seconds..."
		NUM="`expr $NUM - 1`" ; sleep $SLEEPONCE
	done

	if [ "$NUM" = 0 ] || [ -f "$IMAGE.lock" ] ; then
		# TODO: Skip over $IMAGE.md5 verification and use the second-newest file
		logmsg_error "Still locked out of downloading '$IMAGE'"
		case "$ATTEMPT_DOWNLOAD" in
			yes) die "Can not download, so bailing out" ;;
			no) ;;
			*) logmsg_info "Switching from ATTEMPT_DOWNLOAD value '$ATTEMPT_DOWNLOAD' to 'no'" \
				"and skipping OS image '$IMAGE' from candidates to mount"
			   ATTEMPT_DOWNLOAD=no
			   IMAGE_SKIP="$IMAGE"
			   IMAGE=""
			   ;;
		esac
	fi
fi

if [ "$ATTEMPT_DOWNLOAD" = yes ] || [ "$ATTEMPT_DOWNLOAD" = auto ] ; then
	echo "$$" > "$IMAGE.lock"

	settraps 'cleanup_wget; cleanup_script;'

	wget -q -c "$IMAGE_URL.md5" || true
	wget -c "$IMAGE_URL"
	WGET_RES=$?
	logmsg_info "Download completed with exit-code: $WGET_RES"
	if ! ensure_md5sum "$IMAGE" "$IMAGE.md5" ; then
		IMAGE=""
		WGET_RES=127
	fi
	[ "$WGET_RES" = 0 ] || \
		logmsg_error "Failed to get the latest image file name from OBS" \
			"(subsequent VM startup can use a previously downloaded image, however)"

	sync
	cleanup_wget
	settraps 'cleanup_script;'
else
	logmsg_info "Not even trying to download a new OS image at this moment (ATTEMPT_DOWNLOAD=$ATTEMPT_DOWNLOAD)"
fi

if [ x"$DOWNLOADONLY" = xyes ]; then
	logmsg_info "DOWNLOADONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now with exit-code '$WGET_RES'" >&2
	exit $WGET_RES
fi

cd "/srv/libvirt/snapshots" || \
	die "Can not 'cd /srv/libvirt/snapshots' to proceed"
if [ "$1" ]; then
	# TODO: We should not get to this point in current code structure
	# (CLI parsing loop above would fail on unrecognized parameter).
	# Should think if anything must be done about picking a particular
	# image name for a particular VM, though.
	logmsg_info "Using pre-downloaded image filename provided by the user"
	IMAGE="$1"
	# IMAGE_FLAT is used as a prefix to directory filenames of mountpoints
	IMAGE_FLAT="`basename "$IMAGE"`"
else
	if [ -n "$IMAGE" ] && [ -s "$IMGTYPE/$ARCH/$IMAGE" ]; then
		logmsg_info "Recent download succeeded, checksums passed - using this image"
		IMAGE="$IMGTYPE/$ARCH/$IMAGE"
	else
		# If download failed or was skipped, we can have a previous
		# image file for this type
		logmsg_info "Selecting newest image (as sorted by alphabetic name)"
		IMAGE=""
		ls -1 $IMGTYPE/$ARCH/*.$EXT >/dev/null || \
			die "No downloaded image of type $IMGTYPE/$ARCH was found!"
		while [ -z "$IMAGE" ]; do
			if [ -z "$IMAGE_SKIP" ]; then
				IMAGE="`ls -1 $IMGTYPE/$ARCH/*.$EXT | sort -r | head -n 1`"
			else
				IMAGE="`ls -1 $IMGTYPE/$ARCH/*.$EXT | sort -r | grep -v "$IMAGE_SKIP" | head -n 1`"
			fi
			ensure_md5sum "$IMAGE" "$IMAGE.md5" || IMAGE=""
		done
	fi
	IMAGE_FLAT="`basename "$IMAGE" .$EXT`_${IMGTYPE}_${ARCH}.$EXT"
fi
if [ -z "$IMAGE" ]; then
	die "No downloaded image files located in my cache (`pwd`/$IMGTYPE/$ARCH/*.$EXT)!"
fi
if [ ! -s "$IMAGE" ]; then
	die "No downloaded image files located in my cache (`pwd`/$IMAGE)!"
fi
logmsg_info "Will use IMAGE='$IMAGE' for further VM set-up (flattened to '$IMAGE_FLAT')"

# Destroy whatever was running, if anything
virsh -c lxc:// destroy "$VM" 2> /dev/null > /dev/null || \
	logmsg_warn "Could not destroy old instance of '$VM'"
# may be wait for slow box
sleep 5

# Cleanup of the rootfs
logmsg_info "Unmounting paths related to VM '$VM':" \
	"'`pwd`/../rootfs/$VM/lib/modules', '`pwd`../rootfs/$VM/root/.ccache'" \
	"'`pwd`/../rootfs/$VM'/proc, `pwd`/../rootfs/$VM', '`pwd`/../rootfs/${IMAGE_FLAT}-ro'"
umount -fl "../rootfs/$VM/lib/modules" 2> /dev/null > /dev/null || true
umount -fl "../rootfs/$VM/root/.ccache" 2> /dev/null > /dev/null || true
umount -fl "../rootfs/$VM/proc" 2> /dev/null > /dev/null || true
umount -fl "../rootfs/$VM" 2> /dev/null > /dev/null || true
fusermount -u -z  "../rootfs/$VM" 2> /dev/null > /dev/null || true

# This unmount can fail if for example several containers use the same RO image
# or if it is not used at all; not shielding by "$OVERLAYFS" check just in case
umount -fl "../rootfs/${IMAGE_FLAT}-ro" 2> /dev/null > /dev/null || true

# Destroy the overlay-rw half of the old running container, if any
if [ -d "../overlays/${IMAGE_FLAT}__${VM}" ]; then
	logmsg_info "Removing RW directory of the stopped VM:" \
		"'../overlays/${IMAGE_FLAT}__${VM}'"
	rm -rf "../overlays/${IMAGE_FLAT}__${VM}"
	rm -rf "../overlays/${IMAGE_FLAT}__${VM}.tmp"
	sleep 1; echo ""
fi

# When the host gets ungracefully rebooted, useless old dirs may remain...
for D in ../overlays/*__${VM}/ ../overlays/*__${VM}.tmp/ ; do
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
	[ x"$D" = x"../rootfs/${IMAGE_FLAT}-ro/" ] && continue
	# Now, ignore non-directories and not-empty dirs (used mountpoints)
	if [ -d "$D" ]; then
		# This is a directory
		if FD="`cd "$D" && pwd`" && \
			[ x"`mount | grep ' on '${FD}' type '`" != x ] \
		; then
			# This is an active mountpoint... is anything overlaid?
			mount | egrep 'lowerdir=('"`echo ${D} | sed 's,/$,,g'`|${FD}),upperdir=" && \
			logmsg_warn "Old RO mountpoint '$FD' seems still used" && \
			continue

			logmsg_info "Old RO mountpoint '$FD' seems unused, unmounting"
			umount -fl "$FD" || true

			### NOTE: experiments showed, that even if we unmount
			### the RO lowerdir and it is no longer seen by the OS,
			### the overlay mounted filesystem tree remains alive
			### and usable until that overlay is unmounted.
		fi

		if [ x"`cd $D && find .`" = x. ]; then
			# This is a directory, and it is empty
			# Just in case, re-check the mountpoint activity
			FD="`cd "$D" && pwd`" && \
			    [ x"`mount | grep ' on '${FD}' type '`" != x ] && \
			    logmsg_warn "Old RO mountpoint '$FD' seems still used" && \
			    continue

			logmsg_warn "Obsolete RO mountpoint for this IMAGE was found," \
			    "removing '`pwd`/$D':"
			ls -lad "$D"; ls -la "$D"
			umount -fl "$D" 2> /dev/null > /dev/null || true
			rm -rf "$D"
			sleep 1; echo ""
		fi
	fi
done


# clean up VM space
logmsg_info "Removing VM rootfs from '`pwd`/../rootfs/$VM'"
/bin/rm -rf "../rootfs/$VM"

if [ x"$STOPONLY" = xyes ]; then
	logmsg_info "STOPONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now" >&2
	exit 0
fi

logmsg_info "Creating a new VM rootfs at '`pwd`/../rootfs/$VM'"
mkdir -p "../rootfs/$VM"
if [ "$OVERLAYFS" = yes ]; then
	logmsg_info "Mount the common RO squashfs at '`pwd`/../rootfs/${IMAGE_FLAT}-ro'"
	mkdir -p "../rootfs/${IMAGE_FLAT}-ro"
	mount -o loop "$IMAGE" "../rootfs/${IMAGE_FLAT}-ro" || \
		die "Can't mount squashfs"

	logmsg_info "Use the individual RW component" \
		"located in '`pwd`/../overlays/${IMAGE_FLAT}__${VM}'" \
		"for an overlay-mount united at '`pwd`/../rootfs/$VM'"
	mkdir -p "../overlays/${IMAGE_FLAT}__${VM}" \
		"../overlays/${IMAGE_FLAT}__${VM}.tmp"
	mount -t ${OVERLAYFS_TYPE} \
	    -o lowerdir="../rootfs/${IMAGE_FLAT}-ro",upperdir="../overlays/${IMAGE_FLAT}__${VM}",workdir="../overlays/${IMAGE_FLAT}__${VM}.tmp" \
	    ${OVERLAYFS_TYPE} "../rootfs/$VM" \
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

logmsg_info "Bind-mount ccache directory from the host OS"
umount -fl "../rootfs/$VM/root/.ccache" 2> /dev/null > /dev/null || true
# The devel-image can make this a symlink to user homedir, so kill it:
[ -h "../rootfs/$VM/root/.ccache" ] && rm -f "../rootfs/$VM/root/.ccache"
# On some systems this may fail due to strange implementation of overlayfs:
if mkdir -p "../rootfs/$VM/root/.ccache" ; then
	[ -d "/root/.ccache" ] || mkdir -p "/root/.ccache"
	mount -o rbind "/root/.ccache" "../rootfs/$VM/root/.ccache"
fi

logmsg_info "Setup virtual hostname"
echo "$VM" > "../rootfs/$VM/etc/hostname"
logmsg_info "Put virtual hostname in /etc/hosts"
sed -r -i "s/^127\.0\.0\.1/127.0.0.1 $VM /" "../rootfs/$VM/etc/hosts"

logmsg_info "Copy root's ~/.ssh from the host OS"
cp -r --preserve ~/.ssh "../rootfs/$VM/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "../rootfs/$VM/etc/ssh"

logmsg_info "Copy environment settings from the host OS"
cp /etc/profile.d/* ../rootfs/$VM/etc/profile.d/

if [ -d /lib/terminfo/x ] ; then
	logmsg_info "Add xterm terminfo from the host OS"
	mkdir -p ../rootfs/$VM/lib/terminfo/x
	cp -prf /lib/terminfo/x/xterm* ../rootfs/$VM/lib/terminfo/x
fi

mkdir -p "../rootfs/$VM/etc/apt/apt.conf.d/"
# setup debian proxy
[ -n "$APT_PROXY" ] && \
	logmsg_info "Set up APT proxy configuration" && \
	echo 'Acquire::http::Proxy "'"$APT_PROXY"'";' > \
		"../rootfs/$VM/etc/apt/apt.conf.d/01proxy-apt-cacher"

if [ ! -d "../rootfs/$VM/var/lib/mysql/mysql" ] && \
   [ ! -s "../rootfs/$VM/root/.my.cnf" ] && \
   [ -s ~root/.my.cnf ] \
; then
	logmsg_info "Copying MySQL root password from the host into VM"
	cp -pf ~root/.my.cnf "../rootfs/$VM/root/.my.cnf"
fi

if [ -n "${COPYHOST_GROUPS-}" ]; then
	for G in $COPYHOST_GROUPS ; do
		_G="$(egrep "^$G:" "/etc/group")" || \
			die "Can not replicate unknown group '$G' from the host!"

		logmsg_info "Defining group account '$_G' from host to VM"
		if egrep "^$G:" "../rootfs/$VM/etc/group" ; then
			egrep -v "^$G:" < "../rootfs/$VM/etc/group" > "../rootfs/$VM/etc/group.tmp" && \
			echo "$_G" >> "../rootfs/$VM/etc/group.tmp" && \
			cat "../rootfs/$VM/etc/group.tmp" > "../rootfs/$VM/etc/group"
			rm -f "../rootfs/$VM/etc/group.tmp"
		else
			echo "$_G" >> "../rootfs/$VM/etc/group"
		fi
	done
fi

if [ -n "${COPYHOST_USERS-}" ]; then
	for U in $COPYHOST_USERS ; do
		_P="$(egrep "^$U:" "/etc/passwd")" || \
			die "Can not replicate unknown user '$U' from the host!"

		_S="$(egrep "^$U:" "/etc/shadow")" || \
			_S="$U:*:16231:0:99999:7:::"

		logmsg_info "Defining user account '$_P' from host to VM"
		if egrep "^$U:" "../rootfs/$VM/etc/passwd" ; then
			egrep -v "^$U:" < "../rootfs/$VM/etc/passwd" > "../rootfs/$VM/etc/passwd.tmp" && \
			echo "$_P" >> "../rootfs/$VM/etc/passwd.tmp" && \
			cat "../rootfs/$VM/etc/passwd.tmp" > "../rootfs/$VM/etc/passwd"
			rm -f "../rootfs/$VM/etc/passwd.tmp"
		else
			echo "$_P" >> "../rootfs/$VM/etc/passwd"
		fi

		if egrep "^$U:" "../rootfs/$VM/etc/shadow" ; then
			egrep -v "^$U:" < "../rootfs/$VM/etc/shadow" > "../rootfs/$VM/etc/shadow.tmp" && \
			echo "$_S" >> "../rootfs/$VM/etc/shadow.tmp" && \
			cat "../rootfs/$VM/etc/shadow.tmp" > "../rootfs/$VM/etc/shadow"
			rm -f "../rootfs/$VM/etc/shadow.tmp"
		else
			echo "$_S" >> "../rootfs/$VM/etc/shadow"
		fi
	done
fi

logmsg_info "Pre-configuration of VM '$VM' ($IMGTYPE/$ARCH) is completed"
if [ x"$DEPLOYONLY" = xyes ]; then
	logmsg_info "DEPLOYONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now with exit-code '0'" >&2
	[ "$INSTALL_DEV_PKGS" = yes ] && \
		logmsg_info "Note that INSTALL_DEV_PKGS was requested - it is hereby skipped" >&2
	exit 0
fi

logmsg_info "Start the virtual machine $VM"
virsh -c lxc:// start "$VM" || die "Can't start the virtual machine $VM"

if [ "$INSTALL_DEV_PKGS" = yes ]; then
	INSTALLER=""
	if [ -s "$SCRIPTPWD/ci-setup-test-machine.sh" ] ; then
		INSTALLER="$SCRIPTPWD/ci-setup-test-machine.sh"
	else
		if [ -s "$SCRIPTDIR/ci-setup-test-machine.sh" ] ; then
			INSTALLER="$SCRIPTDIR/ci-setup-test-machine.sh"
		else
			[ -n "$CHECKOUTDIR" ] && \
			[ -s "$CHECKOUTDIR/tests/CI/ci-setup-test-machine.sh" ] && \
				INSTALLER="$CHECKOUTDIR/tests/CI/ci-setup-test-machine.sh"
		fi
	fi
	if [ -n "$INSTALLER" ] ; then
		echo "Will now update and install a predefined development package set using $INSTALLER"
		echo "Sleeping 30 sec to let VM startup settle down first..."
		sleep 30
		echo "Running $INSTALLER against the VM '$VM' (via chroot)..."
		chroot ../rootfs/$VM/ /bin/sh < "$INSTALLER"
		echo "Result of installer script: $?"
	else
		echo "WARNING: Got request to update and install a predefined" \
			"development package set, but got no ci-setup-test-machine.sh" \
			"around - action skipped"
	fi
fi

logmsg_info "Preparation and startup of the virtual machine '$VM'" \
	"is successfully completed on `date -u` on host" \
	"'`hostname`${DOTDOMAINNAME}'"

cleanup_script || true
settraps '-'
exit 0
