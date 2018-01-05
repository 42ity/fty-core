#!/bin/bash
#
# Copyright (C) 2014-2016 Eaton
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

PATH="/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:$PATH"
export PATH

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
	echo "    -qa|--qa-level type  image QA level to use (Default: '$IMGQALEVEL')"
	echo "                         see Jenkins and OBS for supported types (master, pre-rc, rc)"
	echo "    -r|--repository URL  OBS image repo ('$OBS_IMAGES')"
	echo "    -hp|--http-proxy URL the http_proxy override to access OBS ('$http_proxy')"
	echo "    -ap|--apt-proxy URL  the http_proxy to access external APT images ('$APT_PROXY')"
	echo "    --install-dev        run ci-setup-test-machine.sh (if available) to install packages"
	echo "    --no-install-dev     do not run ci-setup-test-machine.sh, even on IMGTYPE=devel"
	echo "    --no-restore-saved   do not copy stashed custom configs from a VMNAME.saved/ dir"
	echo "    --no-overlayfs       enforce use of tarballs, even if overlayfs is supported by host"
	echo "    --with-overlayfs     enforce use of overlayfs, fail if not supported by host"
	echo "    --download-only      end the script after downloading the newest image file"
	echo "    --attempt-download [auto|yes|no] Should an OS image download be attempted at all?"
	echo "                         (default: auto; default if only the option is specified: yes)"
	echo "    --no-download        Alias to '--attempt-download no' to redeploy an existing image"
	echo "    --stop-only          end the script after stopping the VM and cleaning up"
	echo "    --destroy-only       end the script after stopping the VM (container 'destroy')"
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
		logmsg_warn "Removing broken file: '$1'"
		rm -f "$1" "$1.md5" "$2"
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

probe_mounts() {
	# $1 = VM name
	# For customized containers that might bind-mount directories provided
	# by the host (and maybe network-backed directories with autofs timeouts
	# involved), startup of a container can return errors like this:
	#   error: internal error: guest failed to start:
	#       Failure in libvirt_lxc startup:
	#       Failed to bind mount directory X to Y:
	#       Too many levels of symbolic links
	# The next attempt usually succeeds. The recommended solution is to list
	# their contents and so "prime" the autofs ability to serve them.
	# This routine tries to read *all* configured directories and returns an
	# error if *any* of them failed.
	virsh dumpxml "$1" | \
	grep '<source dir=' | sed "s|^.*<source dir='\(/[^\']*\)'[ /].*>.*$|\1|" | \
	(RES=0 ; while read D ; do ls "$D/" > /dev/null || RES=$? ; done; exit $RES)
}

#
# defaults
#
### TODO: Assign this default later in the script, after downloads
VM="latest"
[ -z "$IMGTYPE" ] && IMGTYPE="devel"
### Prefix and suffix around $IMGTYPE in the URL directory component
### and the image filename (at the moment, at least, applies to both)
[ -z "$IMGTYPE_PREFIX" ] && IMGTYPE_PREFIX=""
[ -z "$IMGTYPE_SUFFIX" ] && IMGTYPE_SUFFIX="-image"

### "master" is the initial stage for X86 testing, no longer built on ARM
### "pre-rc" is what's tested by Jenkins (on ARM too)
### "rc" is what passed the tests by Jenkins
[ -z "$IMGQALEVEL" ] && IMGQALEVEL="master"
#[ -z "$IMGQALEVEL" ] && IMGQALEVEL="pre-rc"
#[ -z "$IMGQALEVEL" ] && IMGQALEVEL="rc"

[ -z "$OBS_IMAGES" ] && OBS_IMAGES="http://tomcat.roz.lab.etn.com/images/"
#[ -z "$OBS_IMAGES" ] && OBS_IMAGES="http://obs.roz.lab.etn.com/images/"
[ -z "$APT_PROXY" ] && APT_PROXY='http://thunderbolt.roz.lab.etn.com:3142'
#[ -z "$APT_PROXY" ] && APT_PROXY='http://gate.roz.lab.etn.com:3142'
[ -n "$http_proxy" ] && export http_proxy

### SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN is a regex of the basename (no ext)
### Defaults are assigned below after CLI processing
# SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN="simpleimage.*"
# FLAG_FLATTEN_FILENAMES=yes

[ -z "$LANG" ] && LANG=C
[ -z "$LANGUAGE" ] && LANGUAGE=C
[ -z "$LC_ALL" ] && LC_ALL=C
[ -z "$TZ" ] && TZ=UTC
export LANG LANGUAGE LC_ALL TZ

DOTDOMAINNAME="`dnsdomainname | grep -v '('`" || \
DOTDOMAINNAME="`domainname | grep -v '('`" || \
DOTDOMAINNAME=""
[ -n "$DOTDOMAINNAME" ] && DOTDOMAINNAME=".$DOTDOMAINNAME"

[ -z "$DESTROYONLY" ] && DESTROYONLY=no	# LXC destroy == domain stopped
[ -z "$STOPONLY" ] && STOPONLY=no		# domain stopped and rootfs wiped (legacy misnomer)
[ -z "$DOWNLOADONLY" ] && DOWNLOADONLY=no
[ -z "$DEPLOYONLY" ] && DEPLOYONLY=no
[ -z "$INSTALL_DEV_PKGS" ] && INSTALL_DEV_PKGS=auto
[ -z "$ATTEMPT_DOWNLOAD" ] && ATTEMPT_DOWNLOAD=auto
[ -z "$ALLOW_CONFIG_FILE" ] && ALLOW_CONFIG_FILE=yes
[ -z "${OVERLAYFS-}" ] && OVERLAYFS="auto"
[ -z "${NO_RESTORE_SAVED-}" ] && NO_RESTORE_SAVED=no

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
	-qa|--qa-level)
		IMGQALEVEL="$2"
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
	--destroy-only)
		DESTROYONLY=yes
		shift
		;;
	--no-overlayfs)
		OVERLAYFS=no
		shift
		;;
	--no-restore-saved)
		NO_RESTORE_SAVED=yes
		shift
		;;
	--with-overlayfs)
		OVERLAYFS=yes
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
			yes|no|auto|pretend) ATTEMPT_DOWNLOAD="$1"; shift ;;
			*) ATTEMPT_DOWNLOAD=yes ;;
		esac
		;;
	--no-download)
		shift
		ATTEMPT_DOWNLOAD=no
		;;
	--dry-run|-n)
		ATTEMPT_DOWNLOAD=pretend
		DOWNLOADONLY=yes
		shift
		;;
	--install-dev|--install-dev-pkgs)
		INSTALL_DEV_PKGS=yes
		# This one is now defined by ci-setup-test-machine.sh
		# since installing more files into DEV environments is
		# no longer default.
		FORCE_RUN_APT=yes
		export FORCE_RUN_APT
		shift
		;;
	--no-install-dev|--no-install-dev-pkgs)
		INSTALL_DEV_PKGS=no
		FORCE_RUN_APT=""
		export FORCE_RUN_APT
		shift
		;;
	--copy-host-users)
		COPYHOST_USERS="$2"; shift 2;;
	--copy-host-groups)
		COPYHOST_GROUPS="$2"; shift 2;;
	--elevate-users)
		ELEVATE_USERS="$2"; shift 2;;
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
[ "$DOWNLOADONLY" != yes ] && [ -z "$VM" ] && die "VM parameter not provided!"
# $VM may be empty if we only want to download
if [ -n "$VM" ]; then
	RESULT=$(virsh -c lxc:// list --all | awk '/^ *[0-9-]+ +'"$VM"' / {print $2}' | wc -l)
	if [ $RESULT = 0 ] ; then
		die "VM $VM does not exist"
	fi
	if [ $RESULT -gt 1 ] ; then
		### Should not get here via CI
		die "VM pattern '$VM' matches too much ($RESULT)"
		### TODO: spawn many child-shells with same parameters, for each VM?
	fi
fi
# If $VM is not empty here, it is trustworthy

# This should not be hit...
[ -z "$APT_PROXY" ] && APT_PROXY="$http_proxy"
[ x"$APT_PROXY" = x- ] && APT_PROXY=""
[ x"$http_proxy" = x- ] && http_proxy="" && export http_proxy

[ -z "$ARCH" ] && ARCH="`uname -m`"
# Note: several hardcoded paths are expected relative to "snapshots", so
# it is critical that we succeed changing into this directory in the end.

mkdir -p "/srv/libvirt/rootfs" "/srv/libvirt/overlays" "/srv/libvirt/overlays-ro"
cd "/srv/libvirt/rootfs" || \
	die "Can not 'cd /srv/libvirt/rootfs' to keep container root trees"

# A per-VM config file might change things like the IMGTYPE
if [ -n "$VM" ] && [ -s "`pwd`/$VM.config-reset-vm" ]; then
	if [ "$ALLOW_CONFIG_FILE" = yes ]; then
		logmsg_warn "Found configuration file for the '$VM', it will override the command-line settings:"
		cat "`pwd`/$VM.config-reset-vm"
		. "`pwd`/$VM.config-reset-vm" || die "Can not import config file '`pwd`/$VM.config-reset-vm'"
	else
		logmsg_warn "Found configuration file for the '$VM', but it is ignored because ALLOW_CONFIG_FILE='$ALLOW_CONFIG_FILE'"
	fi
fi

[ x"$INSTALL_DEV_PKGS" = xauto ] && \
case "$IMGTYPE" in
	*devel*) INSTALL_DEV_PKGS=yes ;;
	*) INSTALL_DEV_PKGS=no ;;
esac

mkdir -p "/srv/libvirt/snapshots/$IMGTYPE/$ARCH"

# Unless these were set by caller or config or somehow else,
# define the values now. This pattern is a REGEX.
[ -z "$SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN" ] && SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN="${IMGTYPE_PREFIX}${IMGTYPE}${IMGTYPE_SUFFIX}"'-.*_'"${ARCH}"
#NOOP:# [ -z "$FLAG_FLATTEN_FILENAMES" ] && FLAG_FLATTEN_FILENAMES=no

# Make sure we have a loop device support
modprobe loop # TODO: die on failure?

case x"$OVERLAYFS" in
xauto|xyes)
	# Do we have overlayfs in kernel?
	if \
		[ "`gzip -cd /proc/config.gz 2>/dev/null | grep OVERLAY`" ] || \
		grep OVERLAY "/boot/config-`uname -r`" >/dev/null 2>/dev/null  \
	; then
		OVERLAYFS="yes"
	else
		[ x"$OVERLAYFS" = xyes ] && die "OVERLAYFS='$OVERLAYFS' set by caller but not supported by kernel"
		OVERLAYFS="no"
	fi
	;;
xno)	logmsg_warn "OVERLAYFS='$OVERLAYFS' set by caller" ;;
*)	logmsg_warn "Unknown OVERLAYFS='$OVERLAYFS' set by caller, assuming 'no'"; OVERLAYFS="no" ;;
esac

case x"$OVERLAYFS" in
xyes)
	EXT="squashfs"
	logmsg_info "Detected support of OVERLAYFS on the host" \
		"`hostname`${DOTDOMAINNAME}, so will mount a .$EXT file" \
		"as an RO base and overlay the RW changes"
	modprobe squashfs
	for OVERLAYFS_TYPE in overlay overlayfs ; do
		modprobe ${OVERLAYFS_TYPE} && break
	done
	;;
xno)
	EXT="tar.gz"
	OVERLAYFS_TYPE=""
	logmsg_info "Detected no support of OVERLAYFS on the host" \
		"`hostname`${DOTDOMAINNAME}, so will unpack a .$EXT file" \
		"into a dedicated full RW directory"
	;;
esac

# Verify that this script runs once at a time for the given VM
if [ -n "$VM" ] && [ -f "$VM.lock" ] ; then
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

if [ -n "$VM" ] ; then
	( echo "$$"; echo "${_SCRIPT_PATH}"; echo "${_SCRIPT_ARGS}" ) > "$VM.lock"
fi
settraps 'cleanup_script'

# Proceed to downloads, etc.
cd "/srv/libvirt/snapshots/$IMGTYPE/$ARCH" || \
	die "Can not 'cd /srv/libvirt/snapshots/$IMGTYPE/$ARCH' to download image files"

# Initial value, aka "file not found"
WGET_RES=127
IMAGE=""
IMAGE_SKIP=""

sort_osimage_names() {
	# ASSUMPTION: we don't have over 999 rebuilds of the same baseline image ;)
	# ASSUMPTION2: all image builds have a rebuild-index suffix for the same
	# baseline, or there is one old image for a baseline without a suffix.
	### sort
	### sort -n
	sed -e 's,-\([[:digit:]][[:digit:]]\.[[:digit:]][[:digit:]]\.[[:digit:]][[:digit:]]\)_,-\1-0_,' \
	    -e 's,-\([[:digit:]]\)_,-0\1_,' \
	    -e 's,-\([[:digit:]][[:digit:]]\)_,-0\1_,' \
	| sort -n | \
	sed -e 's,-0*\([123456789][[:digit:]]*\)_,-\1_,' \
	    -e 's,-00*_,_,'
}

if [ "$ATTEMPT_DOWNLOAD" != no ] ; then
	logmsg_info "Get the latest operating environment image prepared for us by OBS"
	IMAGE_URL="`wget -O - "$OBS_IMAGES/${IMGTYPE_PREFIX}${IMGTYPE}${IMGTYPE_SUFFIX}/${IMGQALEVEL:+$IMGQALEVEL/}${ARCH}/" 2> /dev/null | sed -n 's|.*href="\(.*'"${SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN}"'\.'"$EXT"'\)".*|'"$OBS_IMAGES/${IMGTYPE_PREFIX}${IMGTYPE}${IMGTYPE_SUFFIX}/${IMGQALEVEL:+$IMGQALEVEL/}${ARCH}"'/\1|p' | sed 's,\([^:]\)//,\1/,g' | sort_osimage_names | tail -n 1`"
	[ $? = 0 ] && [ -n "$IMAGE_URL" ] || die "Could not detect remote IMAGE_URL at '$OBS_IMAGES/${IMGTYPE_PREFIX}${IMGTYPE}${IMGTYPE_SUFFIX}/${IMGQALEVEL:+$IMGQALEVEL/}${ARCH}/' (looking for regex '${SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN}')!"
	IMAGE="`basename "$IMAGE_URL"`"
	[ $? = 0 ] && [ -n "$IMAGE" ] || die "Could not detect remote IMAGE_URL at '$OBS_IMAGES/${IMGTYPE_PREFIX}${IMGTYPE}${IMGTYPE_SUFFIX}/${IMGQALEVEL:+$IMGQALEVEL/}${ARCH}/' (looking for regex '${SOURCESITEROOT_OSIMAGE_FILENAMEPATTERN}')!"

	if [ "$ATTEMPT_DOWNLOAD" = pretend ] ; then
		logmsg_info "Detected '$IMAGE_URL' as the newest available remote OS image for IMGTYPE='$IMGTYPE' and IMGQALEVEL='$IMGQALEVEL'"
		[ -s "$IMAGE" ] \
			&& logmsg_info "'`pwd`/$IMAGE' is locally available already" \
			|| logmsg_warn "'`pwd`/$IMAGE' is not yet locally available"
		CODE=0 die "Dry-run done"
	fi

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
	IMAGE_FLAT="`basename "$IMAGE" .$EXT`_${IMGTYPE}_${ARCH}_${IMGQALEVEL}.$EXT"
fi
if [ -z "$IMAGE" ]; then
	die "No downloaded image files located in my cache (`pwd`/$IMGTYPE/$ARCH/*.$EXT)!"
fi
if [ ! -s "$IMAGE" ]; then
	die "No downloaded image files located in my cache (`pwd`/$IMAGE)!"
fi
logmsg_info "Will use IMAGE='$IMAGE' for further VM set-up (flattened to '$IMAGE_FLAT')"

# Should not get here normally
[ -z "$VM" ] && die "Downloads and verifications are completed, at this point I need a definite VM value to work on!"

# Destroy whatever was running, if anything
# Try graceful shutdown first, to avoid remaining processes that hold up FS
logmsg_info "Destroying VM '$VM' instance (if any was running)..."
virsh -c lxc:// shutdown "$VM" || true
virsh -c lxc:// destroy "$VM" || \
	logmsg_warn "Could not destroy old instance of '$VM'"
# may be wait for slow box
sleep 5

# Cleanup of the rootfs
ALTROOT="$(cd "`pwd`/../rootfs/$VM" && pwd)" || die "Could not determine the container ALTROOT"
logmsg_info "Unmounting paths related to VM '$VM':" \
	"'${ALTROOT}/lib/modules', '${ALTROOT}/root/.ccache'" \
	"'${ALTROOT}/proc', '${ALTROOT}', '`pwd`/../rootfs/${IMAGE_FLAT}-ro'," \
	"'`pwd`/../overlays-ro/${IMAGE_FLAT}-ro'"
umount -fl "${ALTROOT}/lib/modules" 2> /dev/null > /dev/null || true
umount -fl "${ALTROOT}/root/.ccache" 2> /dev/null > /dev/null || true
umount -fl "${ALTROOT}/proc" 2> /dev/null > /dev/null || true
umount -fl "${ALTROOT}" 2> /dev/null > /dev/null || true
fusermount -u -z "${ALTROOT}" 2> /dev/null > /dev/null || true

# This unmount can fail if for example several containers use the same RO image
# or if it is not used at all; not shielding by "$OVERLAYFS" check just in case
umount -fl "../overlays-ro/${IMAGE_FLAT}-ro" 2> /dev/null > /dev/null || true
umount -fl "../rootfs/${IMAGE_FLAT}-ro" 2> /dev/null > /dev/null || true

# root bash history may be protected by chattr to be append-only
chattr -a "${ALTROOT}/root/.bash_history" || true

if [ x"$DESTROYONLY" = xyes ]; then
	logmsg_info "DESTROYONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now, after just stopping" >&2
	exit 0
fi

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
for D in ../overlays-ro/*-ro/ ../rootfs/*-ro/ ; do
	# Do not remove the current IMAGE mountpoint if we reuse it again now
	### [ x"$D" = x"../rootfs/${IMAGE_FLAT}-ro/" ] && continue
	[ x"$D" = x"../overlays-ro/${IMAGE_FLAT}-ro/" ] && continue
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
logmsg_info "Removing VM rootfs from '${ALTROOT}'"
if ! /bin/rm -rf "${ALTROOT}" ; then
	logmsg_error "FAILED to remove '${ALTROOT}'"
	logmsg_info "Checking if blocked by any processes?.."
	fuser "${ALTROOT}" "${ALTROOT}"/*
	fuser -c "${ALTROOT}" "${ALTROOT}"/*
	fuser -m "${ALTROOT}" "${ALTROOT}"/*
	die "Can not manipulate '${ALTROOT}' at this time"
fi

if [ x"$STOPONLY" = xyes ]; then
	logmsg_info "STOPONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now, after stopping and wiping" >&2
	exit 0
fi

logmsg_info "Creating a new VM rootfs at '${ALTROOT}'"
mkdir -p "${ALTROOT}"
if [ x"$OVERLAYFS" = xyes ]; then
	logmsg_info "Mount the common RO squashfs at '`pwd`/../overlays-ro/${IMAGE_FLAT}-ro'"
	mkdir -p "../overlays-ro/${IMAGE_FLAT}-ro"
	mount -o loop "$IMAGE" "../overlays-ro/${IMAGE_FLAT}-ro" || \
		die "Can't mount squashfs"

	logmsg_info "Use the individual RW component" \
		"located in '`pwd`/../overlays/${IMAGE_FLAT}__${VM}'" \
		"for an overlay-mount united at '${ALTROOT}'"
	mkdir -p "../overlays/${IMAGE_FLAT}__${VM}" \
		"../overlays/${IMAGE_FLAT}__${VM}.tmp"
	mount -t ${OVERLAYFS_TYPE} \
		-o lowerdir="../overlays-ro/${IMAGE_FLAT}-ro",upperdir="../overlays/${IMAGE_FLAT}__${VM}",workdir="../overlays/${IMAGE_FLAT}__${VM}.tmp" \
		${OVERLAYFS_TYPE} "${ALTROOT}" \
	|| die "Can't overlay-mount rw directory"
else
	logmsg_info "Unpack the full individual RW copy of the image" \
		"'$IMAGE' at '${ALTROOT}'"
	tar -C "${ALTROOT}" -xzf "$IMAGE" \
	|| die "Can't un-tar the rw directory"
fi

case "$IMAGE" in
	/*) OSIMAGE_FILENAME="$IMAGE" ;;
	*)  OSIMAGE_FILENAME="$(cd `dirname "$IMAGE"` && pwd)/$(basename "$IMAGE")"
esac
OSIMAGE_LSINFO="`ls -lad "$OSIMAGE_FILENAME"`" || OSIMAGE_LSINFO=""
if [ -s "$OSIMAGE_FILENAME.md5" ]; then
	OSIMAGE_CKSUM="`cat "$OSIMAGE_FILENAME.md5" | awk '{print $1}'`"
else
	OSIMAGE_CKSUM="`md5sum < "$OSIMAGE_FILENAME" | awk '{print $1}'`"
fi

logmsg_info "Bind-mount kernel modules from the host OS"
mkdir -p "${ALTROOT}/lib/modules"
mount -o rbind "/lib/modules" "${ALTROOT}/lib/modules"
mount -o remount,ro,rbind "${ALTROOT}/lib/modules"

logmsg_info "Bind-mount ccache directory from the host OS"
umount -fl "${ALTROOT}/root/.ccache" 2> /dev/null > /dev/null || true
# The devel-image can make this a symlink to user homedir, so kill it:
[ -h "${ALTROOT}/root/.ccache" ] && rm -f "${ALTROOT}/root/.ccache"
# On some systems this may fail due to strange implementation of overlayfs:
if mkdir -p "${ALTROOT}/root/.ccache" ; then
	[ -d "/root/.ccache" ] || mkdir -p "/root/.ccache"
	mount -o rbind "/root/.ccache" "${ALTROOT}/root/.ccache"
fi

# Some bits might be required in an image early...
# say, an /usr/bin/qemu-arm-static is a nice trick ;)
if [ -d "${ALTROOT}.saved-preinstall/" ] && [ "$NO_RESTORE_SAVED" != yes ]; then
	logmsg_info "Restoring custom configuration from '`cd ${ALTROOT}.saved-preinstall/ && pwd`':" && \
	( cd "${ALTROOT}.saved-preinstall/" && tar cf - . ) | ( cd "${ALTROOT}/" && tar xvf - )
fi

if [ "$INSTALL_DEV_PKGS" = yes ]; then
	logmsg_info "Set up initial name resolution from the host OS to facilitate apt-get for dev package installation"
	chroot "${ALTROOT}/" cp -pf /etc/resolv.conf /etc/resolv.conf.bak-devpkg || true
	chroot "${ALTROOT}/" cp -pf /etc/nsswitch.conf /etc/nsswitch.conf.bak-devpkg || true
	tar cf - /etc/hosts /etc/resolv.conf /etc/nsswitch.conf | chroot "${ALTROOT}/" tar xf -
fi

logmsg_info "Setup virtual hostname"
echo "$VM" > "${ALTROOT}/etc/hostname"
logmsg_info "Put virtual hostname in /etc/hosts"
# Apparently, the first token for a locally available IP address is
# treated as the `hostname --fqdn` if no other ideas are available.
sed -r -i "s/^127\.0\.0\.1/127.0.0.1 $VM /" "${ALTROOT}/etc/hosts"

logmsg_info "Copy root's ~/.ssh from the host OS"
cp -r --preserve ~/.ssh "${ALTROOT}/root/"
cp -r --preserve /etc/ssh/*_key /etc/ssh/*.pub "${ALTROOT}/etc/ssh"
if [ -d "${ALTROOT}/home/admin" ] && \
   [ ! -d "${ALTROOT}/home/admin/.ssh" ] \
; then
	logmsg_info "Copy root's ~/.ssh from the host OS into guest ~admin/.ssh"
	cp -r --preserve ~/.ssh/ "${ALTROOT}/home/admin/.ssh/"
	if _P="$(egrep '^admin:' "${ALTROOT}/etc/passwd")" \
	&& [ -n "$_P" ]; then
		_UG="`echo "$_P" | awk -F: '{print $3":"$4}'`" && \
		[ -n "$_UG" ] && \
		logmsg_info "Chowning guest ~admin/.ssh to $_UG" && \
		chown -R "$_UG" "${ALTROOT}/home/admin/.ssh/"
	fi
fi

if [ -f ~/.oscrc ]; then
	logmsg_info "Copy root's ~/.oscrc from the host OS"
	cp --preserve ~/.oscrc "${ALTROOT}/root/"
fi

if [ -d ~/.config ]; then
	logmsg_info "Copy root's ~/.config/ from the host OS"
	cp --preserve -r ~/.config "${ALTROOT}/root/"
fi

logmsg_info "Copy environment settings from the host OS"
cp /etc/profile.d/* ${ALTROOT}/etc/profile.d/

if [ -d /lib/terminfo/x ] ; then
	logmsg_info "Add xterm terminfo from the host OS"
	mkdir -p ${ALTROOT}/lib/terminfo/x
	cp -prf /lib/terminfo/x/xterm* ${ALTROOT}/lib/terminfo/x
fi

# setup debian proxy
mkdir -p "${ALTROOT}/etc/apt/apt.conf.d/"
[ -n "$APT_PROXY" ] && [ -d "${ALTROOT}/etc/apt/apt.conf.d" ] && \
	logmsg_info "Set up APT proxy configuration" && \
	echo 'Acquire::http::Proxy "'"$APT_PROXY"'";' > \
		"${ALTROOT}/etc/apt/apt.conf.d/01proxy-apt-cacher"

if [ ! -d "${ALTROOT}/var/lib/mysql/mysql" ] && \
   [ ! -s "${ALTROOT}/root/.my.cnf" ] && \
   [ -s ~root/.my.cnf ] \
; then
	logmsg_info "Copying MySQL root password from the host into VM"
	cp -pf ~root/.my.cnf "${ALTROOT}/root/.my.cnf"
fi

if [ -n "${COPYHOST_GROUPS-}" ]; then
	for G in $COPYHOST_GROUPS ; do
		_G="$(egrep "^$G:" "/etc/group")" || \
			die "Can not replicate unknown group '$G' from the host!"

		logmsg_info "Defining group account '$_G' from host to VM"
		if egrep "^$G:" "${ALTROOT}/etc/group" >/dev/null ; then
			egrep -v "^$G:" < "${ALTROOT}/etc/group" > "${ALTROOT}/etc/group.tmp" && \
			echo "$_G" >> "${ALTROOT}/etc/group.tmp" && \
			cat "${ALTROOT}/etc/group.tmp" > "${ALTROOT}/etc/group"
			rm -f "${ALTROOT}/etc/group.tmp"
		else
			echo "$_G" >> "${ALTROOT}/etc/group"
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
		if egrep "^$U:" "${ALTROOT}/etc/passwd" >/dev/null ; then
			egrep -v "^$U:" < "${ALTROOT}/etc/passwd" > "${ALTROOT}/etc/passwd.tmp" && \
			echo "$_P" >> "${ALTROOT}/etc/passwd.tmp" && \
			cat "${ALTROOT}/etc/passwd.tmp" > "${ALTROOT}/etc/passwd"
			rm -f "${ALTROOT}/etc/passwd.tmp"
		else
			echo "$_P" >> "${ALTROOT}/etc/passwd"
		fi

		if egrep "^$U:" "${ALTROOT}/etc/shadow" >/dev/null ; then
			egrep -v "^$U:" < "${ALTROOT}/etc/shadow" > "${ALTROOT}/etc/shadow.tmp" && \
			echo "$_S" >> "${ALTROOT}/etc/shadow.tmp" && \
			cat "${ALTROOT}/etc/shadow.tmp" > "${ALTROOT}/etc/shadow"
			rm -f "${ALTROOT}/etc/shadow.tmp"
		else
			echo "$_S" >> "${ALTROOT}/etc/shadow"
		fi
	done
fi

if [ -n "${ELEVATE_USERS-}" ]; then
	for U in $ELEVATE_USERS ; do
		echo "$U ALL=(ALL) NOPASSWD: ALL" > "${ALTROOT}/etc/sudoers.d/$U"
	done
fi

if [ -s "${ALTROOT}/usr/share/bios-web/git_details.txt" ]; then
	logmsg_info "GIT details of 'bios-core' preinstalled as a package in '$VM':" \
		"$(cat "${ALTROOT}/usr/share/bios-web/git_details.txt")"
fi

if [ -d "${ALTROOT}.saved/" ] && [ "$NO_RESTORE_SAVED" != yes ]; then
	logmsg_info "Restoring custom configuration from '`cd ${ALTROOT}.saved/ && pwd`':" && \
	( cd "${ALTROOT}.saved/" && tar cf - . ) | ( cd "${ALTROOT}/" && tar xvf - )
fi

logmsg_info "Pre-configuration of VM '$VM' ($IMGTYPE/$ARCH) is completed"
if [ x"$DEPLOYONLY" = xyes ]; then
	logmsg_info "DEPLOYONLY was requested, so ending" \
		"'${_SCRIPT_PATH} ${_SCRIPT_ARGS}' now with exit-code '0'" >&2
	[ "$INSTALL_DEV_PKGS" = yes ] && \
		logmsg_warn "Note that INSTALL_DEV_PKGS was requested - it is hereby skipped" >&2
	exit 0
fi

logmsg_info "Start the virtual machine $VM"
probe_mounts "$VM"
virsh -c lxc:// start "$VM" || die "Can't start the virtual machine $VM"

VM_SHOULD_RESTART=no
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
		logmsg_info "Will now update and install a predefined development package set using $INSTALLER"
		logmsg_info "Sleeping 30 sec to let VM startup settle down first..."
		sleep 30
		logmsg_info "Running $INSTALLER against the VM '$VM' (via chroot into '`cd ${ALTROOT}/ && pwd`')..."
		set +e
		# The DHCP client in the container may wipe the resolv.conf if it found nothing on DHCP line
		#[ -s "${ALTROOT}/etc/resolv.conf" ] || \
		cp -pf /etc/resolv.conf "${ALTROOT}/etc/"
		chroot "${ALTROOT}/" /bin/bash < "$INSTALLER"
		logmsg_info "Result of installer script: $?"
		set -e
	else
		logmsg_warn "Got request to update and install a predefined" \
			"development package set, but got no ci-setup-test-machine.sh" \
			"around - action skipped"
	fi

	set +e
	logmsg_info "Restore /etc/hosts and /etc/resolv.conf in the VM to the default baseline"
	if [ -f "${ALTROOT}.saved/etc/hosts" ] && \
	    diff "${ALTROOT}.saved/etc/hosts" "${ALTROOT}/etc/hosts" >/dev/null 2>&1 \
	; then
		logmsg_info "Keeping /etc/hosts in the VM from the 'saved' template"
	else
		LOCALHOSTLINE="`grep '127.0.0.1' "${ALTROOT}/etc/hosts"`" && \
			[ -n "$LOCALHOSTLINE" ] && ( echo "$LOCALHOSTLINE $VM" > "${ALTROOT}/etc/hosts" )
		logmsg_info "Restore /etc/hosts in the VM to the default baseline"
	fi

	if [ -f "${ALTROOT}.saved/etc/resolv.conf" ] && \
	    diff "${ALTROOT}.saved/etc/resolv.conf" "${ALTROOT}/etc/resolv.conf" >/dev/null 2>&1 \
	; then
		logmsg_info "Keeping /etc/resolv.conf in the VM from the 'saved' template"
	else
		logmsg_info "Restore /etc/resolv.conf in the VM to the default baseline"
		grep "8.8.8.8" "${ALTROOT}/etc/resolv.conf.bak-devpkg" >/dev/null || \
			cp -pf "${ALTROOT}/etc/resolv.conf.bak-devpkg" "${ALTROOT}/etc/resolv.conf"
	fi

	if [ -f "${ALTROOT}.saved/etc/nsswitch.conf" ] && \
	    diff "${ALTROOT}.saved/etc/nsswitch.conf" "${ALTROOT}/etc/nsswitch.conf" >/dev/null 2>&1 \
	; then
		logmsg_info "Keeping /etc/nsswitch.conf in the VM from the 'saved' template"
	else
		logmsg_info "Restore /etc/nsswitch.conf in the VM to the default baseline"
		cp -pf "${ALTROOT}/etc/nsswitch.conf.bak-devpkg" "${ALTROOT}/etc/nsswitch.conf"
	fi

#	logmsg_info "Restart networking in the VM chroot to refresh virtual network settings"
#	chroot "${ALTROOT}/" /bin/systemctl restart bios-networking
	set -e
	VM_SHOULD_RESTART=yes
fi

if [ -z "${GEN_REL_DETAILS-}" ] ; then
    GEN_REL_DETAILS=""
    for F in "${ALTROOT}/usr/share/fty/scripts/generate-release-details.sh" \
        "${ALTROOT}/usr/share/bios/scripts/generate-release-details.sh" ; do
            [ -s "$F" ] && [ -x "$F" ] && GEN_REL_DETAILS="$F" && break
    done
fi

if [ -n "${GEN_REL_DETAILS}" -a -s "${GEN_REL_DETAILS}" -a -x "${GEN_REL_DETAILS}" \
]; then (
	export ALTROOT
	export OSIMAGE_FILENAME OSIMAGE_LSINFO OSIMAGE_CKSUM
	# Note: The variables below are not populated on non-target hardware
	# But for tests they might be set in e.g. the container custom config
	# file like  /srv/libvirt/rootfs/bios-deploy.config-reset-vm
	# Note: We do this only *after* possibly installing/updating more
	# packages which may include newer releases of our software components.
	export MODIMAGE_FILENAME MODIMAGE_LSINFO MODIMAGE_CKSUM
	export BIOSINFO_UBOOT_ID_ETN   BIOSINFO_UBOOT_ID_OG    BIOSINFO_UBOOT_TSS
	export BIOSINFO_UIMAGE_ID_ETN  BIOSINFO_UIMAGE_ID_OG   BIOSINFO_UIMAGE_TSS
	export BIOSINFO_UBOOT          BIOSINFO_UIMAGE
	export FW_UBOOTPART_CSDEV      FW_UBOOTPART_BYTES
	export FW_UBOOTPART_CSDEVPAD   FW_UBOOTPART_SIZE
	export FW_UIMAGEPART_CSDEV     FW_UIMAGEPART_BYTES
	export FW_UIMAGEPART_CSDEVPAD  FW_UIMAGEPART_SIZE
	export HWD_CATALOG_NB  HWD_REV HWD_SERIAL_NB

	logmsg_info "Generating the release details file(s) with the ${GEN_REL_DETAILS} script in OS image"
	"${GEN_REL_DETAILS}"
) ; fi

if [ "$VM_SHOULD_RESTART" = yes ]; then
	logmsg_info "Restart the virtual machine $VM"
	virsh -c lxc:// shutdown "$VM" || true
	virsh -c lxc:// destroy "$VM" && sleep 5 && \
	{ probe_mounts "$VM"; virsh -c lxc:// start "$VM"; } || die "Can't reboot the virtual machine $VM"
	logmsg_info "Sleeping 30 sec to let VM startup settle down..."
	sleep 30
fi

logmsg_info "Preparation and startup of the virtual machine '$VM'" \
	"is successfully completed on `date -u` on host" \
	"'`hostname`${DOTDOMAINNAME}'"

cleanup_script || true
settraps '-'
exit 0
