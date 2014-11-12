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
# Author(s): Jim Klimov <EvgenyKlimov@eaton.com>
#
# Description: This script adds the group and user accounts used by
# the $BIOS project, and should work under relocated $ALTROOT dir too.
# You can export ALTROOT_MAKEFAKE=Y to have the script prepare some
# files needed for the user management programs to perform (i.e. to
# generate the snippets in a temporary directory and later copy them
# elsewhere as part of the image generation or installation procedure).
# See also: tests/CI/test_web.sh
# See the variables set below. Can also use DEBUG=Y for more output
# including sensitive data (like the password or its hash).
#
# Note: This should be run as root or with a user that can elevate;
#       for now we just treat errors (that are not "already-exist")
#       as fatal...

# Default settings
DEF_USER_PASS="nosoup4u" && \
DEF_USER_PASS_HASH='$6$F5TqxQBgnjXt$O0ZJbFbGg9aeT9m21OL9YanudVeaMygUVCOt7lzVM.wV7yAHa0u1sfcklGydt9A5gHmGYN0b/Byff3fCJkXBh1'

MKPASSWD="/usr/bin/mkpasswd"
OPENSSL="/usr/bin/openssl"

PATH=/usr/sbin:/sbin:/usr/local/sbin:/usr/bin:/bin:/usr/local/bin:$PATH
export PATH

LANG=C
LC_ALL=C
export LANG LC_ALL

# Currently we support one user and group, but the script is modularized
# to possibly easily generate more accounts in a loop, later...
[ x"$GROUP_NAME" = x ] &&	GROUP_NAME="bios"
[ x"$USER_NAME" = x ] &&	USER_NAME="bios"
[ x"$USER_GECOS" = x ] && 	USER_GECOS="User for BIOS processes"
[ x"$USER_SHELL" = x ] && 	USER_SHELL="/bin/sh"
[ x"$USER_HOME" = x ] &&	USER_HOME="/home/$USER_NAME"
[ x"$USER_PASS" = x -a x"$USER_PASS_HASH" = x ] && \
	USER_PASS="$DEF_USER_PASS" && \
	USER_PASS_HASH="$DEF_USER_PASS_HASH" && \
	echo "INFO: Using the default hardcoded password (or rather its hardcoded hash)"

die() {
    [ x"$CODE" = x ] && CODE=1
    echo "FATAL: `date`: $0: $@" >&2
    exit $CODE
}

# Simply prepend "sudo" if current "id -n" is not "0" (moderately portable)?
# A more elaborate solution if needed later can be ported from vboxsvc:
# http://sourceforge.net/p/vboxsvc/code/HEAD/tree/lib/svc/method/vbox.sh#l337
# http://sourceforge.net/p/vboxsvc/code/HEAD/tree/lib/svc/method/vbox.sh#l1887
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

# An alternate image based at ALTROOT may be modified instead of the running OS
[ x"$ALTROOT" = x ] &&		ALTROOT="/"

# Note this creation of fake root only works if ALTROOT dir is not initialized
if [ x"$ALTROOT_MAKEFAKE" = xY -a x"$ALTROOT" != x/ ]; then
    if [ ! -d "$ALTROOT/etc" ]; then
	echo "INFO: Trying to make a fake altroot structure in '$ALTROOT' as requested..."
	mkdir -p "$ALTROOT/etc"
        ( cd "$ALTROOT/etc" && {
	    mkdir -p default
	    touch passwd shadow group gshadow login.defs default/useradd nsswitch.conf
	    chmod 600 shadow gshadow

	    cat <<EOF>nsswitch.conf
passwd:  files
group:   files
shadow:  files
gshadow: files
EOF

	    # Make a best-effort with these files, though they are not strictly required
	    cd /etc
	    cp -prf login.defs default/useradd \
		ldap* pam* secur* selinux* \
		"$ALTROOT/etc/"
	} )

	if [ ! -d "$ALTROOT/lib" ]; then
	    mkdir -p "$ALTROOT/lib"
	    ( cd /lib && { \
		    find . -name 'libnss*.so*'; \
		    find . -name 'libnsl*.so*'; \
		    find . -name 'libc*.so*'; \
	      } | while read F; do
		D="`dirname "$F"`"
		mkdir -p "$ALTROOT/lib/$D"
		cp -pf "$F" "$ALTROOT/lib/$D/"
	      done )
	fi

    else
	echo "WARNING: Requested to make a fake altroot structure in '$ALTROOT' but it seems to exist, skipped step"
    fi
fi

[ -d "$ALTROOT" -a -d "$ALTROOT/etc" ] || \
	die "Alternate (chroot) OS-image directory requested but not available: '$ALTROOT'"

# We do not fail due to these errors right now, because user accounts may be
# managed in a networked database like LDAP, AD, NIS, etc.:
[ -f "$ALTROOT/etc/passwd" -a -f "$ALTROOT/etc/shadow" -a \
  -f "$ALTROOT/etc/group" -a -f "$ALTROOT/etc/gshadow" -a \
  -f "$ALTROOT/etc/default/useradd" -a \
  -f "$ALTROOT/etc/login.defs" -a -f "$ALTROOT/etc/nsswitch.conf" ] || \
	echo "WARNING: Alternate (chroot) OS-image directory '$ALTROOT' does not contain all expected files: local authentication database manipulation can fail during processing below"

hashPasswd() {
    # Creates a UNIX password hash using $MKPASSWD or $OPENSSL and the
    # value of global $USER_PASS as the plaintext password (no salt now).
    # Stores the result in global envvar $USER_PASS_HASH.
    # Returns 0 if generation was successful (hash not empty).

    # mkpasswd as in debian8 today can generate these hashes:
    # 	sha-512		'$6$'
    #	sha-256		'$5$'
    #	md5    		'$1$'
    #	crypt		'...'
    if [ -x ${MKPASSWD} ]; then
	{ USER_PASS_HASH="`echo "$USER_PASS" | ${MKPASSWD} -s -m sha-512`" && \
	  echo "INFO: Generated password hash with mkpasswd: sha-512" || \
	  USER_PASS_HASH="" ; }

	[ x"$USER_PASS_HASH" = x ] && \
	{ USER_PASS_HASH="`echo "$USER_PASS" | ${MKPASSWD} -s -m sha-256`" && \
	  echo "INFO: Generated password hash with mkpasswd: sha-256" || \
	  USER_PASS_HASH="" ; }

	[ x"$USER_PASS_HASH" = x ] && \
	{ USER_PASS_HASH="`echo "$USER_PASS" | ${MKPASSWD} -s -m md5`" && \
	  echo "INFO: Generated password hash with mkpasswd: md5" || \
	  USER_PASS_HASH="" ; }

	[ x"$USER_PASS_HASH" = x ] && \
	{ USER_PASS_HASH="`echo "$USER_PASS" | ${MKPASSWD} -s -m des`" && \
	  echo "INFO: Generated password hash with mkpasswd: des-56/crypt" || \
	  USER_PASS_HASH="" ; }
    fi

    # openssl as in debian8 today can generate md5 and crypt hashes
    [ x"$USER_PASS_HASH" = x ] && if [ -x ${OPENSSL} ]; then \
	{ USER_PASS_HASH="`${OPENSSL} passwd -1 "$USER_PASS"`" && \
	  echo "INFO: Generated password hash with openssl: md5" || \
	  USER_PASS_HASH="" ; }

	[ x"$USER_PASS_HASH" = x ] && \
	{ USER_PASS_HASH="`${OPENSSL} passwd "$USER_PASS"`" && \
	  echo "INFO: Generated password hash with openssl: crypt" || \
	  USER_PASS_HASH="" ; }
    fi

    # Final verification as the exitcode
    [ x"$USER_PASS_HASH" != x ]
}

genGroup() {
    # Creates a group by name
    echo "INFO: Creating group '$GROUP_NAME'"

    $RUNAS groupadd -R "$ALTROOT" "$GROUP_NAME"
    RES_G=$?
    case "$RES_G" in
	0)   ;;	# added okay
	4|9) RES_G=0 ;;	# not unique name or number
	*)   CODE=$RES_G die "Error during 'groupadd $GROUP_NAME' ($RES_G)" ;;
    esac
    return $RES_G
}

genUser() {
    # Creates a user by name from $USER_NAME, with default group $GROUP_NAME,
    # shell from $USER_SHELL and description/comment field from $USER_GECOS
    # Takes password from $USER_PASS_HASH if avaiable, or generates from
    # $USER_PASS otherwise

    if [ x"$USER_PASS_HASH" = x ]; then
	hashPasswd
    fi

    if [ x"$USER_PASS_HASH" = x ]; then
	echo "WARNING: Could not generate a password hash, falling back to default password" >&2
	[ x"$DEBUG" != x ] && echo "    default password: '$DEF_USER_PASS'" >&2
	USER_PASS_HASH="$DEF_USER_PASS_HASH"
    fi

    [ x"$DEBUG" = x ] && \
	echo "INFO: Creating user:group '$USER_NAME:$GROUP_NAME'" || \
	echo "INFO: Using password hash '$USER_PASS_HASH' for user:group '$USER_NAME:$GROUP_NAME'"

    if [ x"$ALTROOT_MAKEFAKE" = xY -a -d "$ALTROOT" -a x"$ALTROOT" != x/ ]; then
	$RUNAS mkdir -p "$ALTROOT/`dirname "$USER_HOME"`"
    fi

    $RUNAS useradd -g "$GROUP_NAME" -s "$USER_SHELL" \
	-R "$ALTROOT" -c "$USER_GECOS" \
	-m -d "$USER_HOME" \
	-p "$USER_PASS_HASH" \
	"$USER_NAME"
    RES_U=$?
    case "$RES_U" in
	0)   ;;	# added okay
	4|9) RES_U=0
	     echo "WARNING: Account '$USER_NAME' already exists, information (including password) not replaced now!" >&2
	     ;;	# not unique name or number
	*)   CODE=$RES_U die "Error during 'useradd $USER_NAME' ($RES_U)" ;;
    esac
    return $RES_U
}

verifyGU() {
    echo "INFO: Verifying group account:"
    getent group "$GROUP_NAME" || echo "FAIL"

    echo "INFO: Verifying user account:"
    getent passwd "$USER_NAME" || echo "FAIL"
    finger "$USER_NAME" 2>/dev/null
    id "$USER_NAME" && echo "OK"
}

genGroup
genUser
verifyGU
