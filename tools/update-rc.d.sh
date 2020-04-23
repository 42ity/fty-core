#!/bin/bash
#
# update-rc.d	Update the links in /etc/rc[0-9S].d/
# A bash rewrite of the Debian perl-based utility...
# It is not necessarily as efficient, but should work with same logic.
# Rewritten 2016 by Jim Klimov <EvgenyKlimov@eaton.com>
#

#
# Copyright (C) 2016 - 2020 Eaton
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
#! \file    update-rc.d
#  \brief   Helper script to manage SysVinit scripts in Debian systemd
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#

initd="/etc/init.d"
etcd="/etc/rc"
notreally=0

set -o pipefail

# Print usage message and die.

usage() {
    if [[ $# -gt 0 ]]; then
	echo "update-rc.d: error: $*" >&2
    fi

    cat >&2 <<EOF
usage: update-rc.d [-n] [-f] <basename> remove
       update-rc.d [-n] [-f] <basename> defaults
       update-rc.d [-n] <basename> disable|enable [S|2|3|4|5]
		-n: not really
		-f: force

The disable|enable API is not stable and might change in the future.
EOF
	exit 1
}

save_last_action() {
    # No-op (archive removed)
    :
}

remove_last_action() {
    # No-op (archive removed)
    :
}

info() {
    echo "update-rc.d: $*"
}

warning() {
    echo "update-rc.d: warning: $*" >&2
}

error() {
    echo "update-rc.d: error: $*" >&2
    exit 1
}

error_code() {
    rc="$1"
    echo "update-rc.d: error: $*" >&2
    exit $rc
}

make_path() {
    # Not simply "mkdir -p" so we can chmod the levels, as in original
    D="$1"
    [[ -n "$D" ]] && [[ "$D" != "/" ]] || return 0
    [[ -d "$D" ]] && return 0
    B="`basename "$D"`"
    if [[ ! -d "$B" ]] ; then
        make_path "$B" || return $?
    fi
    mkdir "$D" && \
    chmod 755 "$D"
}

# Given a script name, print any runlevels except 0 or 6 in which the
# script is enabled.  If that gives nothing and the script is not
# explicitly disabled, print 6 if the script is disabled in runlevel
# 0 or 6.
script_runlevels() {
    scriptname="$1"
    links="`ls -1d /etc/rc[S12345].d/S[0-9][0-9]$scriptname`" || links=""
    if [[ -n "$links" ]]; then
        echo "$links" | sed 's,^/etc/rc\(.\)\.d/.*$,\1,'
        return 0
    else
        links="`ls -1d /etc/rc[S12345].d/K[0-9][0-9]$scriptname`" || links=""
        if [[ -z "$links" ]]; then
            links="`ls -1d /etc/rc[06].d/K[0-9][0-9]$scriptname`" || links=""
            if [[ -n "$links" ]]; then
                echo "6"
                return 0
            fi
        fi
    fi
    return 1    # Make it easier to see we found nothing suitable
}

# Map the sysvinit runlevel to that of openrc.
openrc_rlconv() {
    while [[ $# -gt 0 ]]; do
        case "$1" in
            S) echo "sysinit";;
            1) echo "recovery";;
            2|3|4|5) echo "default";;
            6) echo "off";;
        esac
    done | sort | uniq
}

#sub openrc_rlconv {
#    my %rl_table = (
#        "S" => "sysinit",
#        "1" => "recovery",
#        "2" => "default",
#        "3" => "default",
#        "4" => "default",
#        "5" => "default",
#        "6" => "off" );
#
#    my %seen; # return unique runlevels
#    return grep !$seen{$_}++, map($rl_table{$_}, @_);
#}

systemd_reload() {
    if [[ -d "/run/systemd/system" ]]; then
        systemctl daemon-reload
    fi
}

# Creates the necessary links to enable/disable the service (equivalent of an
# initscript) in systemd.
make_systemd_links() {
    scriptname="$1"
    action="$2"

    # In addition to the insserv call we also enable/disable the service
    # for systemd by creating the appropriate symlink in case there is a
    # native systemd service. We need to do this on our own instead of
    # using systemctl because systemd might not even be installed yet.
    service_path=""
    if [[ -f "/etc/systemd/system/$scriptname.service" ]]; then
        service_path="/etc/systemd/system/$scriptname.service"
    elif [[ -f "/lib/systemd/system/$scriptname.service" ]]; then
        service_path="/lib/systemd/system/$scriptname.service"
    fi

    if [[ -n "$service_path" ]] ; then
        [[ -r "$service_path" ]] || error "unable to read $service_path"
        cat "$service_path" > /dev/null || error "unable to read $service_path"
        egrep '^[ \t]*WantedBy=' "$service_path" | while IFS="=$IFS" read KEY VALUE ; do
            case "${KEY}" in
                WantedBy)
                    wants_dir="/etc/systemd/system/${VALUE}.wants"
                    service_link="$wants_dir/$scriptname.service"
                    if [[ "$action" = "enable" ]]; then
                        make_path "$wants_dir" && \
                        { rm -f "$service_link"; ln -s "$service_path" "$service_link"; }
                    else
                        [[ -e "$service_link" ]] && rm -f "$service_link"
                    fi
                    ;;
            esac
        done < "$service_path"
    fi
}

# Manage the .override file for upstart jobs, so update-rc.d enable/disable
# work on upstart systems the same as on sysvinit/systemd.
upstart_toggle() {
    scriptname="$1"
    action="$2"

    # This needs to be done by manually parsing .override files instead of
    # using initctl, because upstart might not be installed yet.
    service_path=""
    if [[ -f "/etc/init/$scriptname.conf" ]]; then
        service_path="/etc/init/$scriptname.override"
    fi
    [[ -n "$service_path" ]] || \
        return # zero or not exitcode?

    enabled=1
    overrides=''
    while IFS="" read LINE ; do
        if [[ "$LINE" =~ ^[[:blank:]]*manual[[:blank:]]*$ ]]; then
            enabled=0
        else
            [[ -z "$overrides" ]] \
                && overrides="$LINE" \
                || overrides="$overrides
$LINE"
        fi
    done < "$service_path"

    if [[ "$enabled" = 1 ]] && [[ "$action" = 'disable' ]]; then
        echo "manual" >> "$service_path" || error "unable to write $service_path"
    elif [[ "$enabled" = 0 ]] && [[ "$action" = 'enable' ]]; then
        if [[ -n "$overrides" ]] ; then
            echo "$overrides" > "$service_path.new" || error "unable to write $service_path"
            mv -f "$service_path.new" "$service_path" || error "unable to write $service_path"
        else
            rm -f "$service_path" || error "unable to remove $service_path"
        fi
    fi
}

# Try to determine if initscripts is installed
is_initscripts_installed() {
    # Check if mountkernfs is available. We cannot make inferences
    # using the running init system because we may be running in a
    # chroot
    [[ -f '/etc/init.d/mountkernfs.sh' ]] # Return the test exitcode
}

parse_def_start_stop() {
    script="$1"

    lsb_begin=0
    lsb_end=0
    def_start_lvls=""
    def_stop_lvls=""

    cat "$script" >/dev/null || error "unable to read $script"
    while IFS="" read LINE; do
        case "$LINE" in
            \#\#\#\ BEGIN\ INIT\ INFO)
                lsb_begin=$(($lsb_begin+1)) ;;
            \#\#\#\ END\ INIT\ INFO)
                lsb_end=$(($lsb_end+1)); break ;;
            \#\ Default-Start:*)
                [[ "$lsb_begin" != 0 ]] && [[ "$lsb_end" = 0 ]] && \
                def_start_lvls="`echo "$LINE" | sed 's,^[^:]*:[ \t]*,,'`" ;;
            \#\ Default-Stop:*)
                [[ "$lsb_begin" != 0 ]] && [[ "$lsb_end" = 0 ]] && \
                def_stop_lvls="`echo "$LINE" | sed 's,^[^:]*:[ \t]*,,'`" ;;
        esac
    done < "$script"

    echo "$def_start_lvls"
    echo "$def_stop_lvls"
    return 0
}

lsb_header_for_script() {
    name="$1"

    for file in "/etc/insserv/overrides/$name" "/etc/init.d/$name" \
                      "/usr/share/insserv/overrides/$name" \
    ; do
        [[ -s "$file" ]] && echo "$file" && return 0
    done

    error "cannot find a LSB script for $name"
}

cmp_args_with_defaults() {
    name="$1"
    act="$2"
    orig_argv=( "$@" )
    shift 2

    OUT="`parse_def_start_stop "/etc/init.d/$name"`" || exit $? # error opening file
    # Is there any non-void content?
    [[ -n "`echo "$OUT" | tr -d ' \t\n\r'`" ]] || return 0

    # We expect two lines to come out, empty or not
    lsb_start_ref="`echo "$OUT" | head -1`"
    lsb_stop_ref="`echo "$OUT" | tail -1`"
    unset OUT

    # Helper strings
    arg_str=""
    lsb_str=""
    # Arrays:
    lsb_start_lvls=( $lsb_start_ref )
    lsb_stop_lvls=( $lsb_stop_ref )
    arg_start_lvls=( )
    arg_stop_lvls=( )

    warning "start and stop actions are no longer supported; falling back to defaults"
    [[ "$act" = start ]] && start=1 || start=0
    [[ "$act" = stop ]] && stop=1 || stop=0

    # The legacy part of this program passes arguments starting with
    # "start|stop NN x y z ." but the insserv part gives argument list
    # starting with sequence number (ie. strips off leading "start|stop")
    # Start processing arguments immediately after the first seq number.
    [[ "$act" = "$1" ]] && argi=3 || argi=2

    while [[ "$argi" -lt "${#orig_argv[@]}" ]]; do
        arg="${orig_argv[$argi]}"
        argi=$(($argi+1))

        case "$arg" in
            0|6) start=0; stop=1;;
            start) start=1; stop=0; argi=$(($argi+1)); continue;;
            stop) start=0; stop=1; argi=$(($argi+1)); continue;;
            .) continue;;
        esac

        [[ "$start" != 0 ]] && arg_start_lvls+=( "$arg" )
        [[ "$stop" != 0 ]] && arg_stop_lvls+=( "$arg" )
    done

    rc=0
    if [[ "${#arg_start_lvls[@]}" != "${#lsb_start_lvls[@]}" ]] || \
        [[ "`for V in ${arg_start_lvls[@]}; do echo "$V"; done | sort | uniq`" != \
           "`for V in ${lsb_start_lvls[@]}; do echo "$V"; done | sort | uniq`" ]] \
    ; then
        arg_str="${arg_start_lvls[@]}" && \
            [[ -n "$arg_str" ]] && \
            [[ -n "`echo "$arg_str" | tr -d ' \n\t\r'`" ]] \
        || arg_str="none"
        lsb_str="${lsb_start_lvls[@]}" && \
            [[ -n "$lsb_str" ]] && \
            [[ -n "`echo "$lsb_str" | tr -d ' \n\t\r'`" ]] \
        || lsb_str="none"
        warning "start runlevel arguments ($arg_str) do not match" \
                "$name Default-Start values ($lsb_str)"
        rc=1
    fi

    if [[ "${#arg_stop_lvls[@]}" != "${#lsb_stop_lvls[@]}" ]] || \
        [[ "`for V in ${arg_stop_lvls[@]}; do echo "$V"; done | sort | uniq`" != \
           "`for V in ${lsb_stop_lvls[@]}; do echo "$V"; done | sort | uniq`" ]] \
    ; then
        arg_str="${arg_stop_lvls[@]}" && \
            [[ -n "$arg_str" ]] && \
            [[ -n "`echo "$arg_str" | tr -d ' \n\t\r'`" ]] \
        || arg_str="none"
        lsb_str="${lsb_stop_lvls[@]}" && \
            [[ -n "$lsb_str" ]] && \
            [[ -n "`echo "$lsb_str" | tr -d ' \n\t\r'`" ]] \
        || lsb_str="none"
        warning "stop runlevel arguments ($arg_str) do not match" \
                "$name Default-Stop values ($lsb_str)"
        rc=1
    fi

    return $rc
}

insserv_toggle() {
    dryrun="$1"
    act="$2"
    name="$3"
    shift 3

#    my ($dryrun, $act, $name) = (shift, shift, shift);
#    my (@toggle_lvls, $start_lvls, $stop_lvls, @symlinks);
    lsb_header="`lsb_header_for_script "$name"`" || exit $?

    # Extra arguments to disable|enable action are runlevels. If none
    # given parse LSB info for Default-Start value.
    start_lvls=""
    stop_lvls=""
    if [[ $# -gt 0 ]]; then
        toggle_lvls=( "$@" )
    else
        OUT="`parse_def_start_stop "$lsb_header"`" || exit $? # error opening file
        # Is there any non-void content?
        if [[ -n "`echo "$OUT" | tr -d ' \t\n\r'`" ]]; then
            # We expect two lines to come out, empty or not
            start_lvls="`echo "$OUT" | head -1`"
            stop_lvls="`echo "$OUT" | tail -1`"
        fi
        unset OUT

        [[ -n "`echo "$start_lvls" | tr -d ' \t\n\r'`" ]] || \
            error "$name Default-Start contains no runlevels, aborting."

        toggle_lvls=( $start_lvls )
    fi

    if [[ -x "/sbin/openrc" ]]; then
        case "$act" in
            disable) openrc_act="del" ;;
            enable) openrc_act="add" ;;
            *) openrc_act=""; warning "Unsupported action for openrc: cannot map '$act'";;
        esac
        [[ -z "$openrc_act" ]] || \
            rc-update "$openrc_act" "$name" `openrc_rlconv ${toggle_lvls[@]}`
    fi

    # Find symlinks in rc.d directories. Refuse to modify links in runlevels
    # not used for normal system start sequence.
    symlinks=( )
    for lvl in "${toggle_lvls[@]}"; do
        case "$lvl" in
            S|2|3|4|5)
                for F in /etc/rc${lvl}.d/[SK][0-9][0-9]${name} ; do
                    [[ -e "$F" ]] && symlinks+=( "$F" )
                done
                ;;
            *)
                warning "$act action will have no effect on runlevel $lvl"
                ;;
        esac
    done

    if [[ "${#symlinks[@]}" = 0 ]]; then
        error "no runlevel symlinks to modify, aborting!"
    fi

    # Toggle S/K bit of script symlink.
    # NOTE: This is done so in the original script.
    #       Seems to dishonor numeric SK levels...
    for cur_lnk in "${symlinks[@]}"; do
        [[ -z "$cur_lnk" ]] && warning "Got an empty cur_lnk in synlinks array" && continue
        if [[ "$act" = disable ]]; then
            new_lnk="`echo "$cur_lnk" | sed 's,^\(.*\)/S\([^/]*\)$,\1/K\2,'`"
        else
            new_lnk="`echo "$cur_lnk" | sed 's,^\(.*\)/K\([^/]*\)$,\1/S\2,'`"
        fi
        [[ "$new_lnk" = "$cur_lnk" ]] && continue
        [[ -z "$new_lnk" ]] && warning "Ended up with empty new_lnk from '$cur_lnk'" && continue

        if [[ "$dryrun" != 0 ]]; then
            echo "rename($cur_lnk, $new_lnk)"
            continue
        fi

        # Note: At least once (maybe due to FS error) a move failed in testing.
        # However we could remove and create the symlink. So fall back to this.
        # Consider alternate-root environments, so make the link relative.
        # Also note that while an "mv" might retain a customized symlink that
        # actually points somewhere other than the LSB-located script, we chose
        # to not parse the symlink content in re-creation, but forced pointing
        # at the actual current correct location of the script.
        mv -f "$cur_lnk" "$new_lnk" || \
            { warning "Failed to rename($cur_lnk, $new_lnk)"
              warning "`ls -lad "$new_lnk" "$cur_lnk" 2>/dev/null`"
              warning "Falling back to rm($cur_lnk, $new_lnk)+ln($lsb_header, $new_lnk)"
              rm -f "$new_lnk" ; rm -f "$cur_lnk"
              # Note: rm might fail or not; we care about result of ln in the end
              ln -s -r "$lsb_header" "$new_lnk" || \
              ln -s "../../$lsb_header" "$new_lnk"; } || \
            error "Could not rename SK symlink"
    done

    return 0
}

## Dependency based
insserv_updatercd() {
    args=( "$@" )
    opts=""
    scriptname=""
    action=""
    notreally=0

    orig_argv=( "$@" )

    while [[ $# -gt 0 ]]; do
        case "$1" in
            -n) opts="$opts $1"; notreally=$(($notreally+1)) ;;
            -f) opts="$opts $1";;
            -h|--help) usage "" ;;
            -*) usage "unknown option" ;;
            *) break;;
        esac
        shift
    done

    if [[ $# -lt 2 ]]; then
        usage "not enough arguments"
    fi

    # Add force flag if initscripts is not installed
    # This enables inistcripts-less systems to not fail when a facility is missing
    is_initscripts_installed || opts="-f $opts"

    scriptname="$1"
    action="$2"
    shift 2
    insserv="/usr/lib/insserv/insserv"
    # Fallback for older insserv package versions [2014-04-16]
    [[ -x "/sbin/insserv" ]] && insserv="/sbin/insserv"

    #[[ -x "$insserv" ]] || echo "Warning: rc.d symlinks not being kept up to date because insserv is missing!" >&2

    case "$action" in
        remove)
            [[ -x "/sbin/openrc" ]] && rc-update -qqa delete "$scriptname"
            [[ -x "$insserv" ]] || exit 0

            if [[ -f "/etc/init.d/$scriptname" ]]; then
                "$insserv" $opts -r "$scriptname"
                rc=$?
                if [[ "$rc" = 0 ]] && [[ "$notreally" != 0 ]]; then
                    remove_last_action "$scriptname"
                fi
                if [[ "$rc" != 0 ]]; then
                    error_code "$rc" "insserv rejected the script header"
                fi
                systemd_reload
                exit $rc
            else
                # insserv removes all dangling symlinks, no need to tell it
                # what to look for.
                "$insserv" $opts
                rc=$?
                if [[ "$rc" = 0 ]] && [[ "$notreally" != 0 ]]; then
                    remove_last_action "$scriptname"
                fi
                if [[ "$rc" != 0 ]]; then
                    error_code "$rc" "insserv rejected the script header"
                fi
                systemd_reload
                exit $rc
            fi
            ;;
        defaults|start|stop)
            [[ -x "$insserv" ]] || exit 0

            # All start/stop/defaults arguments are discarded, so emit a
            # message if arguments have been given and are in conflict
            # with Default-Start/Default-Stop values of LSB comment.
            if [[ "start" = "$action" ]] || [[ "stop" = "$action" ]]; then
                cmp_args_with_defaults "$scriptname" "$action" "$@"
            fi

            if [[ -f "/etc/init.d/$scriptname" ]]; then
                "$insserv" $opts "$scriptname"
                rc=$?
                if [[ "$rc" = 0 ]] && [[ "$notreally" != 0 ]]; then
                    save_last_action "$scriptname" `echo ${orig_argv[@]}`
                fi
                if [[ "$rc" != 0 ]]; then
                    error_code "$rc" "insserv rejected the script header"
                fi
                systemd_reload

                # OpenRC does not distinguish halt and reboot.  They are handled
                # by /etc/init.d/transit instead.
                if [[ -x "/sbin/openrc" ]] && [[ "halt" != "$scriptname" ]] \
                     && [[ "reboot" != "$scriptname" ]] ; then
                    # no need to consider default disabled runlevels
                    # because everything is disabled by openrc by default
                    rls="`script_runlevels "$scriptname"`" || rls=""
                    [[ -n "$rls" ]] && \
                        rc-update add "$scriptname" `openrc_rlconv $rls`
                fi
                exit $rc
            else
                error "initscript does not exist: /etc/init.d/$scriptname"
            fi
            ;;
        disable|enable)
            make_systemd_links "$scriptname" "$action"

            upstart_toggle "$scriptname" "$action"

            [[ -x "$insserv" ]] || exit 0

            insserv_toggle "$notreally" "$action" "$scriptname" "$@"
            # Call insserv to resequence modified links
            "$insserv" $opts "$scriptname"
            rc=$?
            if [[ "$rc" = 0 ]] && [[ "$notreally" != 0 ]]; then
                save_last_action "$scriptname" `echo ${orig_argv[@]}`
            fi
            if [[ "$rc" != 0 ]]; then
                error_code "$rc" "insserv rejected the script header"
            fi
            systemd_reload

            exit $rc
            ;;
        *) usage "" ;;
    esac
}

insserv_updatercd "$@"
exit $?
