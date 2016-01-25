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
#! \file   testlib-nut.sh
#  \brief  Library used in CI tests involving NUT
#  \author Barbora Stepankova <BarboraStepankova@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Yves Clauzel <ClauzelYves@Eaton.com>
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#
# Note: Lots of authors since code for this lib was picked from older
# standalone scripts, did not bother to research who did what exactly.
#
# Requirements:
# * Our common libraries (scriptlib, testlib, maybe testlib-db, weblib)
#   should be included into the test-scripts before this one.
# * Shell interpreter is expected to be BASH or have compatible syntax.
# * Caller must define a `custom_create_ups_dev_file()` that would
#   accept UPS (path)name as "$1" and echo a dummy-ups config snippet;
#   likewise caller must define `custom_create_epdu_dev_file()`.
#

# NUT options
[ -n "${NUTCFGDIR-}" ] || NUTCFGDIR=""
[ -n "${NUTUSER-}" ] || NUTUSER="nut"
[ -n "${NUTPASSWORD-}" ] || NUTPASSWORD="secret"
detect_nut_cfg_dir() {
    for cfgd in "/etc/ups" "/etc/nut"; do
        if [[ -d "$cfgd" ]] ; then
            NUTCFGDIR="$cfgd"
            break
        fi
    done
    [[ -n "$NUTCFGDIR" ]] && [[ -d "$NUTCFGDIR" ]] || \
        return $?
    return 0
}

set_value_in_ups() {
    local UPS="$(basename "$1" .dev)"
    local PARAM="$2"
    local VALUE="$3"
    local RES=0

    logmsg_debug "set_value_in_ups('$UPS' '$PARAM' '$VALUE')..."
    egrep '^'"$PARAM"' *:' <"$NUTCFGDIR/$UPS.dev" >/dev/null || \
        logmsg_warn "Parameter '$PARAM' is not set in file '$NUTCFGDIR/$UPS.dev'"

    sed -r -e "s/^$PARAM *:.+"'$'"/$PARAM: $VALUE/i" <"$NUTCFGDIR/$UPS.dev" >"$NUTCFGDIR/$UPS.new" && \
    egrep '^'"$PARAM"' *: *'"$VALUE"' *$' <"$NUTCFGDIR/$UPS.new" >/dev/null && \
    mv -f "$NUTCFGDIR/$UPS.new" "$NUTCFGDIR/$UPS.dev" || \
        { RES=$?; logmsg_error "set_value_from_ups() could not generate '$NUTCFGDIR/$UPS.dev' with new '$PARAM=$VALUE' setting"; }

    case "$UPS" in
        ""|@*) logmsg_error "set_value_from_ups() got no reasonable UPS parameter ('$UPS')"; return 1 ;;
        *@*) ;;
        *)   UPS="$UPS@localhost" ;;
    esac

    upsrw -s "$PARAM=$VALUE" -u "$NUTUSER" -p "$NUTPASSWORD" "$UPS" >/dev/null || \
        { logmsg_warn "set_value_from_ups() could not upsrw the '$PARAM=$VALUE' setting onto '$UPS' in real-time";
          [ "$RES" = 0 ] && sleep 2; }
        # The "sleep" above allows dummy-ups to roll around the end of file
        # and propagate the setting, if it is available in that config file
    [ "$RES" = 0 ] && logmsg_debug "set_value_in_ups('$UPS' '$PARAM' '$VALUE') - OK"
    return $RES
}

get_value_from_ups() {
    local UPS="$(basename "$1" .dev)"
    local PARAM="$2"
    logmsg_debug "get_value_from_ups('$UPS' '$PARAM')..."
    RES=0
    case "$UPS" in
        ""|@*) logmsg_error "get_value_from_ups() got no reasonable UPS parameter ('$UPS')"; return 1 ;;
        *@*) ;;
        *)   UPS="$UPS@localhost" ;;
    esac
    VALUE="`upsc "$UPS" "$PARAM"`" || \
        { RES=$?; logmsg_error "get_value_from_ups() could not upsc the '$PARAM' setting from '$UPS'"; }
    [ "$RES" = 0 ] && logmsg_debug "get_value_from_ups('$UPS' '$PARAM') - got '$VALUE' - OK"
    echo "$VALUE"
    return $RES
}

# Before using these wrappers, caller must define custom_create_ups_dev_file()
# and custom_create_epdu_dev_file() !!!
create_ups_dev_file() {
    local FILE="$1"
    logmsg_debug "create_ups_dev_file($FILE)"
    ( custom_create_ups_dev_file "$@" ) \
        > "$FILE" \
        || CODE=$? logmsg_error "create_ups_dev_file($FILE) FAILED ($?)"
}

create_epdu_dev_file() {
    local FILE="$1"
    logmsg_debug "create_epdu_dev_file($FILE)"
    ( custom_create_epdu_dev_file "$@" ) \
        > "$FILE" \
        || CODE=$? logmsg_error "create_epdu_dev_file($FILE) FAILED ($?)"
}

list_nut_devices() {
    awk '/^\[.+\]/{ print substr($0,2,index($0,"]") - 2); }' < "$NUTCFGDIR/ups.conf"
}

have_nut_target() {
    local STATE="`systemctl show nut-driver.target | egrep '^LoadState=' | cut -d= -f2`"
    if [ "$STATE" = "not-found" ] ; then
        echo N
        return 1
    else
        echo Y
        return 0
    fi
}

stop_nut() {
    if [ "$(have_nut_target)" = Y ] ; then
        logmsg_info "Stopping NUT server via systemctl using nut-driver@ instances" >&2
        systemctl stop nut-server
        systemctl stop "nut-driver@*"
        systemctl disable "nut-driver@*"
    else
        logmsg_info "Stopping NUT server via systemctl using monolithic nut-driver" >&2
        systemctl stop nut-server.service
        systemctl stop nut-driver.service
    fi
    sleep 3
}

start_nut() {
    local ups
    local F
    for F in ups.conf upsmon.conf ; do
        [ -s "$NUTCFGDIR/$F" ] && [ -r "$NUTCFGDIR/$F" ] || \
            logmsg_warn "start_nut(): '$NUTCFGDIR/$F' should be a non-empty readable file"
    done
    if [ "$(have_nut_target)" = Y ] ; then
        logmsg_info "Starting NUT server via systemctl using nut-driver@ instances" >&2
        for ups in $(list_nut_devices) ; do
            systemctl enable "nut-driver@$ups" || return $?
            systemctl start "nut-driver@$ups" || return $?
        done
        sleep 3
        systemctl start nut-server || return $?
    else
        logmsg_info "Starting NUT server via systemctl using nut-driver@ instances" >&2
        systemctl start nut-driver.service || return $?
        sleep 3
        systemctl start nut-server.service || return $?
    fi
    sleep 3
}

create_nut_config() {
    local DUMMY_UPSES="${1-}"; shift    # space-separated list
    local DUMMY_EPDUS="${1-}"; shift
    if [ $# -gt 0 ] && [ x"${1-}" != x-- ]; then
        die "Bad call to create_nut_config('$DUMMY_UPSES', '$DUMMY_EPDUS'; eval '$*')!"
    fi
    shift
    # If any more params follow, they will be eval'ed after we generate
    # the standard configs and before we chown files and restart services

    local DEV
    local RES
    stop_nut || true

    test_it "create_nut_config"

    RES=0
    echo "MODE=standalone" > "$NUTCFGDIR/nut.conf" || \
        die "Can not tweak 'nut.conf'"

    { for DEV in $DUMMY_UPSES ; do
        logmsg_debug "Creating config snippet for device: dummy UPS: $DEV"
        create_ups_dev_file "$NUTCFGDIR/$DEV.dev" && \
            logmsg_debug "create_ups_dev_file '$NUTCFGDIR/$DEV.dev' - OK" || RES=$?
        echo -e \
            "[$DEV]" \
            "\ndriver=dummy-ups" \
            "\nport=$DEV.dev" \
            "\ndesc=\"dummy-ups UPS in dummy mode\"" \
            "\n"
      done
      for DEV in $DUMMY_EPDUS ; do
        logmsg_debug "Creating config snippet for device: dummy ePDU: $DEV"
        create_epdu_dev_file "$NUTCFGDIR/$DEV.dev" && \
            logmsg_debug "create_epdu_dev_file '$NUTCFGDIR/$DEV.dev' - OK" || RES=$?
        echo -e \
            "[$DEV]" \
            "\ndriver=dummy-ups" \
            "\nport=$DEV.dev" \
            "\ndesc=\"dummy-ups ePDU in dummy mode\"" \
            "\n"
      done
      } > "$NUTCFGDIR/ups.conf" || \
        die "Can not tweak 'ups.conf'"

    echo -e \
        "[$NUTUSER]" \
        "\npassword=$NUTPASSWORD" \
        "\nactions=SET" \
        "\ninstcmds=ALL" \
        > "$NUTCFGDIR/upsd.users" || \
        die "Can not tweak 'upsd.users'"

    if [ $# -gt 0 ]; then
        logmsg_debug "Calling custom implementation snippet under create_nut_config(): $*"
        eval "$@"
    fi

    chown nut:root "$NUTCFGDIR/"*.dev
    start_nut || RES=$?
    logmsg_info "Waiting for a while after applying NUT config"
    sleep 10
    print_result $RES
    return $RES
}

