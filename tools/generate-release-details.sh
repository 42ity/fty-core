#!/bin/sh
#
# Copyright (C) 2014-2019 Eaton
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
#! \file    generate-release-details.sh
#  \brief   Helper script to embed OS image release details into deployment
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details The pre-built OS images with product delivery are instantiated
#           as individual deployments by various scripts for different
#           technology platforms (most commonly as squashfs and overlayfs
#           for chrooting on ARM target appliances, or as tar.gz containers
#           for X86 development and testing). Those scripts which prepare
#           the individual deployment are expected to call this script with
#           certain exported environment variables, so it would generate the
#           /etc/release-details(.json) and /etc/issue(.net) file contents
#           in a uniform manner regardless of the platform (availability of
#           certain variable values varies from platform to platform though).
#           This code is expected to work in any standard shell interpreter.
#

LANG=C
LC_ALL=C
TZ=UTC
export LANG LC_ALL TZ

# We'd manipulate filesystem trees located under this $ALTROOT base
[ -z "${ALTROOT-}" ] && ALTROOT="/"

# The full list of required variables is presented below
# (required in the sense that they can not be guessed
# from "inside" the image; they may be absent or empty
# if e.g. not applicable to a particular platform) :
#   OSIMAGE_FILENAME    Full path to the OS image on host environment or before chroot
#   OSIMAGE_LSINFO      Output of `ls -la $OSIMAGE_FILENAME`
#   OSIMAGE_CKSUM       (MD5) Checksum of $OSIMAGE_FILENAME
#   MODIMAGE_FILENAME   Full path to the modules archive on host
#   MODIMAGE_LSINFO     Output of `ls -la $MODIMAGE_FILENAME`
#   MODIMAGE_CKSUM      (MD5) Checksum of $MODIMAGE_FILENAME
#   BIOSINFO_UBOOT_ID_ETN   BIOSINFO_UBOOT_ID_OG    BIOSINFO_UBOOT_TSS
#   FW_UBOOTPART_CSDEV      FW_UBOOTPART_BYTES
#   FW_UBOOTPART_CSDEVPAD   FW_UBOOTPART_SIZE
#                       Details about u-Boot loader for this system
#   BIOSINFO_UIMAGE_ID_ETN  BIOSINFO_UIMAGE_ID_OG   BIOSINFO_UIMAGE_TSS
#   FW_UIMAGEPART_CSDEV     FW_UIMAGEPART_BYTES
#   FW_UIMAGEPART_CSDEVPAD  FW_UIMAGEPART_SIZE
#                       Details about uImage miniroot for this system
#   HWD_VENDOR HWD_CATALOG_NB HWD_REV HWD_SERIAL_NB
#                       OEM details about hardware for this system
# For now (code to be ported) we also expect pre-parsed markup in
#   BIOSINFO_UBOOT          BIOSINFO_UIMAGE


# Echo helper
DEBUG_X=""
v_echo() {
        set +x
        echo
        while [ "$1" ]; do
                echo " *** $1 ***"
                shift
        done
        echo
        [ -z "$DEBUG_X" ] || set $DEBUG_X
}

cut1() {
        # Prints the first column from input (used in chopping md5sum output)
        while read S F; do echo "$S"; done
}

v_echo_ts() {
        set +x
        if [ "$KMSG_USED" = yes ] ; then
                v_echo "$@"
        else
                _ARG1="$1"; shift
                v_echo "[`cut1 < /proc/uptime`] ${_ARG1}" "$@"
        fi
        [ -z "$DEBUG_X" ] || set $DEBUG_X
}

# On x86_64, we retrieve the hardware info from SMBIOS, unless it has been
# provided via the environment
if test -d /sys/class/dmi/id; then
        test -n "$HWD_VENDOR" || HWD_VENDOR="$(cat /sys/class/dmi/id/sys_vendor)"
        test -n "$HWD_CATALOG_NB" || HWD_CATALOG_NB="$(cat /sys/class/dmi/id/product_name)"
        test -n "$HWD_REV" || HWD_REV="$(cat /sys/class/dmi/id/product_version)"
        # VMware says "None" here
        HWD_REV="${HWD_REV#None}"
        test -n "$HWD_SERIAL_NB" || HWD_SERIAL_NB="$(cat /sys/class/dmi/id/product_serial)"
        # Ignore VMware serial numbers as per product requirement
        case "$HWD_SERIAL_NB" in
        VMware*)
                HWD_SERIAL_NB=''
                ;;
        esac
        # XXX: Find an SMBIOS equivalent for HWD_PART_NB=
fi

# Basename of $OSIMAGE_FILENAME without archiving extension
[ -z "${OSIMAGE_NAME-}" ] && [ -n "${OSIMAGE_FILENAME-}" ] && \
    OSIMAGE_NAME="`basename "$OSIMAGE_FILENAME" | sed 's/\.\(squashfs\|tar\|tar\..*\|tgz\|tbz2\|txz\)$//'`"

# ? OSIMAGE_BTS         Build timestamp of OS image (can be found inside)
# ? OSIMAGE_TYPE        IMGTYPE of OS image (can be found inside)

if [ -z "${GIT_DETAILS_FILE-}" ]; then
        for F in \
            "${ALTROOT}/usr/share/fty/.git_details" \
            "${ALTROOT}/usr/share/bios/.git_details" \
        ; do
                [ -s "$F" ] && GIT_DETAILS_FILE="$F" && break
        done
fi

BIOSINFO="Readonly base OS image: `basename "$OSIMAGE_FILENAME"`
`. "${GIT_DETAILS_FILE}" >/dev/null 2>&1 && echo "42ity 'core' version:   $PACKAGE_GIT_HASH_S_ESCAPED @ $PACKAGE_GIT_TSTAMP_ISO8601_ESCAPED"`"

OSIMAGE_BTS=""
OSIMAGE_TYPE=""
if [ -s "${ALTROOT}/usr/share/bios-web/image-version.txt" ]; then
        # We expect timestamp built in specific format by "LANG=C date -R -u"
        # and maybe prefixed by the "OSimage:build-ts:" tag to cut out,
        # and similarly a "OSimage:img-type: (string)" if at all present.
        OSIMAGE_BTS="`head -1 "${ALTROOT}/usr/share/bios-web/image-version.txt" | sed 's/^.*\(..., \)/\1/'`" || OSIMAGE_BTS=""
        if [ -n "$OSIMAGE_BTS" ]; then
                OSIMAGE_BTSS="`chroot "${ALTROOT}" /bin/date -u -d "$OSIMAGE_BTS" '+%Y%m%dT%H%M%SZ'`" && \
                        [ -n "$OSIMAGE_BTSS" ] && OSIMAGE_BTS="$OSIMAGE_BTSS"
        fi 2>/dev/null

        OSIMAGE_TYPE="`head -2 "${ALTROOT}/usr/share/bios-web/image-version.txt" | tail -1 | (read TAG TAIL ; echo "$TAIL")`" || OSIMAGE_TYPE=""
fi

if [ -z "${OSIMAGE_TYPE-}" ] && [ -s /etc/update-rc3.d/image-os-type.conf ] ; then
        OSIMAGE_TYPE="`egrep '^OSIMAGE_TYPE=' "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" | sed -e 's,^OSIMAGE_TYPE=,,' -e 's,^"\(.*\)"$,\1,' -e "s,^'\(.*\)'"'$,\1,'`"
        if [ -z "$OSIMAGE_TYPE" ] ; then
                # Note: suffix and prefix may validly be defined and empty, or not defined at all
                _IP="`egrep '^IMGTYPE_PREFIX=' "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" | sed -e 's,^IMGTYPE_PREFIX=,,' -e 's,^"\(.*\)"$,\1,' -e "s,^'\(.*\)'"'$,\1,'`"
                _IT="`egrep '^IMGTYPE=' "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" | sed -e 's,^IMGTYPE=,,' -e 's,^"\(.*\)"$,\1,' -e "s,^'\(.*\)'"'$,\1,'`"
                _IS="`egrep '^IMGTYPE_SUFFIX=' "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" | sed -e 's,^IMGTYPE_SUFFIX=,,' -e 's,^"\(.*\)"$,\1,' -e "s,^'\(.*\)'"'$,\1,'`"
                [ -z "${_IT}" ] || OSIMAGE_TYPE="${_IP}${_IT}${_IS}"
                unset _IP _IT _IS
        fi
fi

WEBUI_ID=""
WEBUI_TS=""
WEBUI_BTS=""
if [ -s "${ALTROOT}/usr/share/bios-web/version.txt" ]; then
        WEBUI_TS="`head -1 "${ALTROOT}/usr/share/bios-web/version.txt"`" || WEBUI_TS=""
        case "$WEBUI_TS" in
            *,*)# Here we expect an older-style timestamp built in specific
                # format by "LANG=C date -R -u" and maybe prefixed by the
                # "bios-web:commit-ts:" tag to be cut out...
                WEBUI_TSS="`echo "$WEBUI_TS" | sed 's/^.*\(..., \)/\1/'`" && \
                [ -n "$WEBUI_TSS" ] && WEBUI_TS="$WEBUI_TSS" && \
                WEBUI_TSS="`chroot "${ALTROOT}" /bin/date -u -d "$WEBUI_TS" '+%Y%m%dT%H%M%SZ'`" && \
                        [ -n "$WEBUI_TSS" ] && WEBUI_TS="$WEBUI_TSS"
                ;;
            *[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9])
                # Here we expect a digits-only YYYYMMDDHHMMSS maybe prefixed
                WEBUI_TS="`echo "$WEBUI_TS" | sed 's/^.*\([0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]\)\([0-9][0-9][0-9][0-9][0-9][0-9]\)[ \t]*$/\1T\2Z/'`"
                ;;
            *)  ;; # Keep as is, empty or not
        esac 2>/dev/null
        [ -z "$WEBUI_TS" ] && WEBUI_TS="timestamp:N/A"
        # We expect long commit ID as the last word, maybe prefixed by
        # "bios-web:commit-id:" tag to cut out in newer releases...
        WEBUI_ID="`head -2 "${ALTROOT}/usr/share/bios-web/version.txt" | tail -1 | sed 's,^.*[ \t]\([^ \t]*\)$,\1,' | sed 's,^\(.......\).*$,\1,'`" || WEBUI_ID="commitID:N/A"

        WEBUI_BTS="`head -3 "${ALTROOT}/usr/share/bios-web/version.txt" | tail -1`" || WEBUI_BTS=""
        case "$WEBUI_BTS" in
            *,*)WEBUI_TSS="`echo "$WEBUI_BTS" | sed 's/^.*\(..., \)/\1/'`" && \
                [ -n "$WEBUI_TSS" ] && WEBUI_BTS="$WEBUI_TSS" && \
                WEBUI_TSS="`chroot "${ALTROOT}" /bin/date -u -d "$WEBUI_BTS" '+%Y%m%dT%H%M%SZ'`" && \
                        [ -n "$WEBUI_TSS" ] && WEBUI_BTS="$WEBUI_TSS"
                ;;
            *[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9])
                # Here we expect a digits-only YYYYMMDDHHMMSS maybe prefixed
                WEBUI_BTS="`echo "$WEBUI_BTS" | sed 's/^.*\([0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]\)\([0-9][0-9][0-9][0-9][0-9][0-9]\)[ \t]*$/\1T\2Z/'`"
                ;;
            *)  ;; # Keep as is, empty or not
        esac 2>/dev/null
        [ -z "$WEBUI_BTS" ] && WEBUI_BTS="timestamp:N/A"

        BIOSINFO="$BIOSINFO
42ity Web-UI version:   $WEBUI_ID @ $WEBUI_TS"
fi

[ -n "$BIOSINFO_UBOOT" ] && \
        BIOSINFO="$BIOSINFO
$BIOSINFO_UBOOT"

[ -n "$BIOSINFO_UIMAGE" ] && \
        BIOSINFO="$BIOSINFO
$BIOSINFO_UIMAGE"

[ -n "$HWD_VENDOR$HWD_CATALOG_NB$HWD_REV$HWD_SERIAL_NB" ] && \
        BIOSINFO="$BIOSINFO
Hardware details: Vendor:$HWD_VENDOR CatalogNumber:$HWD_CATALOG_NB HWSpecRevision:$HWD_REV SerialNumber:$HWD_SERIAL_NB"

# The device/container/VM UUID may be provided by caller somehow, e.g.
# it might come from virtualization infrastructure (plug /proc/cmdline?)
# Otherwise we generate it from whatever unique data we have.
if [ -z "${UUID_VALUE-}" ]; then
    [ -n "${UUID_NAMESPACE-}" ] || UUID_NAMESPACE="3aac7e03-aa86-8b7e-dab6-7021ed8de397"
    # printf '42ity' | sha1sum | sed 's,^\(........\)\(....\)\(....\)\(....\)\(............\).*$,\1-\2-\3-\4\-\5,'

    UUID_VALUE="00000000-0000-0000-0000-000000000000"
    for UUID_PROG in "${ALTROOT}/usr/bin/uuid" /usr/bin/uuid "`which uuid >/dev/null 2>&1`" ; do
        [ -x "$UUID_PROG" ] && break
    done
    if [ -n "$UUID_PROG" ] && [ -x "$UUID_PROG" ] ; then
        UUID_VALUE="$("$UUID_PROG" -v5 "$UUID_NAMESPACE" "$HWD_VENDOR""$HWD_CATALOG_NB""$HWD_SERIAL_NB")" 2>/dev/null || \
        case "$UUID_PROG" in
            "${ALTROOT}/"*) UUID_PROG="`echo "$UUID_PROG" | sed 's,^'"${ALTROOT}"'/,/,'`" && \
                UUID_VALUE="$(chroot "${ALTROOT}" "$UUID_PROG" -v5 "$UUID_NAMESPACE" "$HWD_VENDOR""$HWD_CATALOG_NB""$HWD_SERIAL_NB")" || \
                UUID_VALUE="00000000-0000-0000-0000-000000000000" ;;
            *)  UUID_VALUE="00000000-0000-0000-0000-000000000000" ;;
        esac
    else
        echo "WARNING: the uuid program is not available" >&2
    fi
fi

[ -n "$UUID_VALUE" ] && \
        BIOSINFO="$BIOSINFO
Device UUID: $UUID_VALUE"

if [ -z "${OSIMAGE_DISTRO-}" ]; then
    [ -s "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" ] \
    && OSIMAGE_DISTRO="`egrep '^OSIMAGE_DISTRO=' "${ALTROOT}/etc/update-rc3.d/image-os-type.conf" | sed -e 's,^OSIMAGE_DISTRO=,,' -e 's,^"\(.*\)"$,\1,' -e "s,^'\(.*\)'"'$,\1,'`" \
    && [ -n "$OSIMAGE_DISTRO" ] \
    || OSIMAGE_DISTRO="Debian_8.0" # Legacy default from before we considered OS revisions
fi
BIOSINFO="$BIOSINFO
Bundled into OS distribution: $OSIMAGE_DISTRO"

rm -f "${ALTROOT}/etc/release-details.json" "${ALTROOT}/etc/release-details" || true
# Remove the legacy one only if a file - do not rewrite the symlink to new location needlessly
[ -s "${ALTROOT}/etc/bios-release.json" ] && rm -f "${ALTROOT}/etc/bios-release.json" || true
[ -s "${ALTROOT}/etc/bios-release" ] && rm -f "${ALTROOT}/etc/bios-release" || true

cat <<EOF > "${ALTROOT}/etc/release-details.json"
{ "release-details": {
        "osimage-lsinfo":       "$OSIMAGE_LSINFO",
        "osimage-filename":     "$OSIMAGE_FILENAME",
        "osimage-name":         "$OSIMAGE_NAME",
        "osimage-cksum":        "$OSIMAGE_CKSUM",
        "osimage-build-ts":     "$OSIMAGE_BTS",
        "osimage-img-type":     "$OSIMAGE_TYPE",
        "osimage-distro":       "$OSIMAGE_DISTRO",
        "modimage-lsinfo":      "$MODIMAGE_LSINFO",
        "modimage-filename":    "$MODIMAGE_FILENAME",
        "modimage-cksum":       "$MODIMAGE_CKSUM",
`. "${GIT_DETAILS_FILE}" >/dev/null 2>&1 && printf '\t"bios-core-commit-id":\t"%s",\n\t"bios-core-commit-ts":\t"%s",\n' "$PACKAGE_GIT_HASH_S_ESCAPED" "$PACKAGE_GIT_TSTAMP_ISO8601_ESCAPED" `
        "bios-web-commit-id":   "$WEBUI_ID",
        "bios-web-commit-ts":   "$WEBUI_TS",
        "bios-web-build-ts":    "$WEBUI_BTS",
        "uboot-commit-id-eaton":        "$BIOSINFO_UBOOT_ID_ETN",
        "uboot-commit-id-opengear":     "$BIOSINFO_UBOOT_ID_OG",
        "uboot-build-ts":       "$BIOSINFO_UBOOT_TSS",
        "uboot-part-cksum":     "$FW_UBOOTPART_CSDEV",
        "uboot-part-bytes":     "$FW_UBOOTPART_BYTES",
        "uboot-padded-cksum":   "$FW_UBOOTPART_CSDEVPAD",
        "uboot-padded-bytes":   "$FW_UBOOTPART_SIZE",
        "uimage-commit-id-eaton":       "$BIOSINFO_UIMAGE_ID_ETN",
        "uimage-commit-id-opengear":    "$BIOSINFO_UIMAGE_ID_OG",
        "uimage-build-ts":      "$BIOSINFO_UIMAGE_TSS",
        "uimage-part-cksum":    "$FW_UIMAGEPART_CSDEV",
        "uimage-part-bytes":    "$FW_UIMAGEPART_BYTES",
        "uimage-padded-cksum":  "$FW_UIMAGEPART_CSDEVPAD",
        "uimage-padded-bytes":  "$FW_UIMAGEPART_SIZE",
        "hardware-vendor":      "$HWD_VENDOR",
        "hardware-catalog-number":      "$HWD_CATALOG_NB",
        "hardware-part-number": "$HWD_PART_NB",
        "hardware-spec-revision":       "$HWD_REV",
        "hardware-serial-number":       "$HWD_SERIAL_NB",
        "uuid":        "$UUID_VALUE"
} }
EOF
if [ $? = 0 ] ; then
    if [ ! -s "${ALTROOT}/etc/bios-release.json" ]; then
        rm -f "${ALTROOT}/etc/bios-release.json" || true
        ln -s release-details.json "${ALTROOT}/etc/bios-release.json" || true
    fi

    v_echo_ts "Updated ${ALTROOT}/etc/release-details.json with OS image and 42ity build data"
fi

# Similar data in a more human-readable file
if printf '%s\n%s\n%s\n\n' "$OSIMAGE_LSINFO" "$MODIMAGE_LSINFO" "$BIOSINFO" \
    > "${ALTROOT}/etc/release-details" \
; then
    if [ ! -s "${ALTROOT}/etc/bios-release" ]; then
        rm -f "${ALTROOT}/etc/bios-release" || true
        ln -s release-details "${ALTROOT}/etc/bios-release" || true
    fi
    v_echo_ts "Updated ${ALTROOT}/etc/release-details with OS image and 42ity build data"
fi

# Post these identification data into file(s) of the installed OS RW root
# NOTE: The files in original OS images (squashfs or tarball variants)
# have certain contents in the first line ONLY; the rest is appended by
# this script, and rewritten upon updates.
for F in issue issue.net ; do
    if [ -s "${ALTROOT}/etc/$F" ] ; then
        # Use printf() below to not mangle the issue(5) markup
        # as if it were full of shell variables
        ORIGDATA="`head -1 "${ALTROOT}/etc/$F"`" && \
        printf '%s\n%s\n\n' "$ORIGDATA" "$BIOSINFO" > "${ALTROOT}/etc/$F" && \
            v_echo_ts "Updated ${ALTROOT}/etc/$F with OS image and 42ity build data"
    fi
done
