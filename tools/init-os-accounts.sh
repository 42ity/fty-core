#!/bin/sh
# Note: this script intentionally uses the dumbest-common-denominator
# shell syntax with hopes to work under any arbitrary system shell.
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
#! \file    init-os-accounts.sh
#  \brief   Initialize group and user accounts used by the 42ity project
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details This script adds the group and user accounts used by
#  the 42ity project, and should work under relocated $ALTROOT dir too.
#  You can export ALTROOT_MAKEFAKE=Y to have the script prepare some
#  files needed for the user management programs to perform (i.e. to
#  generate the snippets in a temporary directory and later copy them
#  elsewhere as part of the image generation or installation procedure).
#  The CREATE_HOME=yes envvar or setting in logins.def is honoured.
#  See also: tests/CI/test_web.sh
#  See the variables set below. Can also use DEBUG=Y for more output
#  including sensitive data (like the password or its hash).
#  Also can `export USER_PASS_STDIN` with anon-empty value so the
#  password value would be read from stdin (no leak of envvar).
#
#  \note This should be run as root or with a user that can elevate;
#        for now we just treat errors (that are not "already-exist")
#        as fatal...

# Default settings
DEF_USER_PASS="admin"
DEF_USER_PASS_HASH='$6$uhrwSjVBa33$1h2cGPecZQfJdSd4MbC4KSv1vsp1eSqpJ5O/FqVLPWQysX732Icn6yz8/l72cQJZcFp9OnUnmEBqgr072.hY21'

MKPASSWD="/usr/bin/mkpasswd"
OPENSSL="/usr/bin/openssl"

PATH=/usr/sbin:/sbin:/usr/local/sbin:/usr/bin:/bin:/usr/local/bin:$PATH
export PATH

LANG=C
LC_ALL=C
export LANG LC_ALL

log_info() {
    echo "INFO: $*" >&2
}

log_warning() {
    echo "WARNING: $*" >&2
}

die() {
    [ x"$CODE" = x ] && CODE=1
    echo "FATAL: `date -u`: $0: $@" >&2
    exit $CODE
}

# Currently we support one user and group, but the script is modularized
# to possibly easily generate more accounts in a loop, later...
# Also note that (currently) specific numeric ID's are not enforced.
[ x"$GROUP_NAME" = x ] &&	GROUP_NAME="bios"
[ x"$USER_NAME" = x ] &&	USER_NAME="admin"
[ x"$USER_GECOS" = x ] && 	USER_GECOS="User for 42ity processes"
[ x"$USER_SHELL" = x ] && 	USER_SHELL="/bin/sh"
[ x"$USER_HOME" = x ] &&	USER_HOME="/home/$USER_NAME"
if [ -n "${USER_PASS_STDIN-}" ]; then
    # STDIN overrides the variable even if one is provided
    read USER_PASS
fi
# Do not allow empty password from any source
[ x"$USER_PASS" = x -a x"$USER_PASS_HASH" = x ] && \
    USER_PASS="$DEF_USER_PASS" && \
    USER_PASS_HASH="$DEF_USER_PASS_HASH" && \
    log_info "Using the default hardcoded password (or rather its hardcoded hash)"

# (Optional) additional groups, a space-separated list
[ x"$USER_ADD_GROUPS" = x ] &&	USER_ADD_GROUPS="sasl"
[ x"$USER_ADD_GROUPS" = x- ] &&	USER_ADD_GROUPS=""

# TODO: Perhaps check if "pwgen" is installed and use it to generate
# long and random passwords not to be used by humans?.. `pwgen -sncy 32 1`

# Simply prepend "sudo" if current "id -n" is not "0" (moderately portable)?
# A more elaborate solution if needed later can be ported from vboxsvc:
# http://sourceforge.net/p/vboxsvc/code/HEAD/tree/lib/svc/method/vbox.sh#l337
# http://sourceforge.net/p/vboxsvc/code/HEAD/tree/lib/svc/method/vbox.sh#l1887
RUNAS=""
CURID="`id -u`" || CURID=""
[ "$CURID" = 0 ] || RUNAS="sudo"

# An alternate image based at ALTROOT may be modified instead of the running OS
[ x"$ALTROOT" = x ] &&	        ALTROOT="/"

# Note this creation of fake root only works if ALTROOT dir is not initialized
if [ x"$ALTROOT_MAKEFAKE" = xY -a x"$ALTROOT" != x/ ]; then
    if [ ! -d "$ALTROOT/etc" ]; then
        log_info "Trying to make a fake altroot structure in '$ALTROOT' as requested..."
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
                ldap* pam* secur* selinux* skel \
                "$ALTROOT/etc/"
        } )

        if [ ! -d "$ALTROOT/lib" ]; then
            log_warning "ALTROOT lacks /lib/ - trying to put some needed files in it, but maybe passwd-tools will misbehave on a system different from our reference!"
            sleep 2
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
        log_warning "Requested to make a fake altroot structure in '$ALTROOT' but it seems to exist, skipped step"
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
        log_warning "Alternate (chroot) OS-image directory '$ALTROOT' does not contain all expected files: local authentication database manipulation can fail during processing below"

hashPasswd_mkpasswd() {
    ALGO="$1"
    ALGO_DESCR="$2"
    [ -z "$ALGO_DESCR" ] && ALGO_DESCR="$ALGO"

    { # Do not `echo|cmd` to avoid password leaks here:
      USER_PASS_HASH="`${MKPASSWD} -s -m ${ALGO} << EOF
${USER_PASS}
EOF`" && \
      log_info "Generated password hash with mkpasswd: ${ALGO_DESCR}" || \
      USER_PASS_HASH="" ; }
}

hashPasswd_openssl() {
    ALGO="$1"   # For openssl, can be empty to use default
    ALGO_DESCR="$2"
    [ -z "$ALGO_DESCR" ] && ALGO_DESCR="$ALGO"
    { # Do not `echo|cmd` to avoid password leaks here:
      USER_PASS_HASH="`${OPENSSL} passwd -stdin ${ALGO} << EOF
${USER_PASS}
EOF`" && \
      log_info "Generated password hash with openssl: ${ALGO_DESCR}" || \
      USER_PASS_HASH="" ; }
}

hashPasswd() {
    # Creates a UNIX password hash using $MKPASSWD or $OPENSSL and the
    # value of global $USER_PASS as the plaintext password (no salt now).
    # Stores the result in global envvar $USER_PASS_HASH.
    # Returns 0 if generation was successful (hash not empty).

    # mkpasswd as in debian8 today can generate these hashes:
    #	sha-512		'$6$'
    #	sha-256		'$5$'
    #	md5    		'$1$'
    #	crypt  		'...'
    if [ -x ${MKPASSWD} ]; then
        hashPasswd_mkpasswd "sha-512"
        [ x"$USER_PASS_HASH" = x ] && hashPasswd_mkpasswd "sha-256"
        [ x"$USER_PASS_HASH" = x ] && hashPasswd_mkpasswd "md5"
        [ x"$USER_PASS_HASH" = x ] && hashPasswd_mkpasswd "des" "des-56/crypt"
    fi

    # openssl as in debian8 today can generate md5 and crypt hashes
    [ x"$USER_PASS_HASH" = x ] && if [ -x ${OPENSSL} ]; then \
        hashPasswd_openssl "-1" "md5"
        [ x"$USER_PASS_HASH" = x ] && hashPasswd_openssl "-crypt" "crypt"
    fi

    # Final verification as the exitcode
    [ x"$USER_PASS_HASH" != x ]
}

genGroup() {
    # Creates a group by name
    log_info "Creating group '$GROUP_NAME'"

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
        log_warning "Could not generate a password hash, falling back to default password"
        [ x"$DEBUG" != x ] && echo "    default password: '$DEF_USER_PASS'" >&2
        USER_PASS_HASH="$DEF_USER_PASS_HASH"
    fi

    [ x"$DEBUG" = x ] && \
        log_info "Creating user:group '$USER_NAME:$GROUP_NAME'" || \
        log_info "Using password hash '$USER_PASS_HASH' for user:group '$USER_NAME:$GROUP_NAME'"

    MKHOME_FLAG=""
    [ -f "$ALTROOT/etc/login.defs" ] && \
        egrep '^CREATE_HOME ([Yy]|[Yy][Ee][Ss]|true|on)$' \
            "$ALTROOT/etc/login.defs" > /dev/null && \
        MKHOME_FLAG="-m"

    case "$CREATE_HOME" in
        [Yy]|[Yy][Ee][Ss]|true|on)	MKHOME_FLAG="-m" ;;
    esac

    if [ x"$ALTROOT_MAKEFAKE" = xY -a -d "$ALTROOT" -a \
         x"$ALTROOT" != x/ -a x"$MKHOME_FLAG" != x ]; then
        _HOME_BASE="$ALTROOT/`dirname "$USER_HOME"`"
        [ -d "$_HOME_BASE" ] || \
            $RUNAS mkdir -p "$_HOME_BASE"
    fi

    $RUNAS useradd -g "$GROUP_NAME" -s "$USER_SHELL" \
        -R "$ALTROOT" -c "$USER_GECOS" \
        $MKHOME_FLAG -d "$USER_HOME" \
        -p "$USER_PASS_HASH" \
        "$USER_NAME"
    RES_U=$?
    case "$RES_U" in
        0)   ;;	# added okay
        4|9) RES_U=0
             log_warning "Account '$USER_NAME' already exists, information (including password) not replaced now!"
             ;;	# not unique name or number
        12)  RES_U=0 ;;	# can't create home directory
        *)   CODE=$RES_U die "Error during 'useradd $USER_NAME' ($RES_U)" ;;
    esac

    # Try to add the account into secondary groups such as "sasl",
    # but don't die if this fails
    for G in $USER_ADD_GROUPS ; do
        log_info "Try to add '$G' as a secondary group for '$USER_NAME' (may fail)..."
        $RUNAS usermod -G "$G" -a "$USER_NAME"
    done

    return $RES_U
}

verifyGU() {
    RES=1

    if [ "$ALTROOT" = / ]; then
        log_info "Verifying group account:"
        getent group "$GROUP_NAME" || echo "FAIL"

        log_info "Verifying user account:"
        getent passwd "$USER_NAME" || echo "FAIL" && RES=0
        finger "$USER_NAME" 2>/dev/null
        id "$USER_NAME" && echo "OK" >&2 && RES=0
    else
        log_info "Verifying group account (in ALTROOT):"
        egrep "^$GROUP_NAME" "$ALTROOT/etc/group" || echo "FAIL"

        log_info "Verifying user account (in ALTROOT):"
        egrep "^$USER_NAME" "$ALTROOT/etc/passwd" || echo "FAIL" && RES=0
    fi >&2

    return $RES
}

usage() {
    echo "Create a user account (maybe in an OS image under ALTROOT):
Usage: USER_NAME=... GROUP_NAME=... ALTROOT=... USER_PASS='...' $0
   or: USER_PASS_HASH='...' USER_NAME=..... $0
   or: echo 'S0mePa\$\$' | USER_PASS_STDIN=true USER_NAME=..... $0
See the script code for envvars (a lot!) that you can define to specify the user details

Generate and echo a password hash using one of algorithms present in this OS:
Usage: USER_PASS='...' $0 hashPasswd
   or: echo 'S0mePa\$\$' | USER_PASS_STDIN=true $0 hashPasswd
   or most securely from shell:
USER_PASS_STDIN=true $0 hashPasswd << EOF
S0mePa\$\$
EOF
"
}

if [ $# = 0 ]; then
    genGroup
    genUser
    verifyGU
else
    case "$1" in
        help|-h|--help|-help)
            usage
            exit 0 ;;
        genGroup|genUser|verifyGU)
            [ -z "$DEBUG" = yes ] && die "Unknown args"
            eval "$@"
            exit $? ;;
        hashPasswd*)
            eval "$@" >&2
            RES=$?
            echo "${USER_PASS_HASH}"
            exit $RES ;;
        *) die "Unknown args" ;;
    esac
fi
