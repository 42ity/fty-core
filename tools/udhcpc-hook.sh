#!/bin/bash
#
# Copyright (c) 2010 Debian
# Copyright (c) 2015 - 2020 Eaton
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
#! \file    udhcpc-hook.sh
#  \brief   NTP synchronization and hostname setup for udhcpc
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Based on /etc/dhcp/dhclient-exit-hooks.d/ntp from Debian8 ntp
#           package, adapted for the 42ity project.
#           Included from (patched) /etc/udhcpc/default.script for 42ity project

NTP_CONF=/etc/ntp.conf
NTP_DHCP_CONF=/var/lib/ntp/ntp.conf.dhcp

# The systemd service unit can be "disabled" or "masked" if users want a
# manually set clock - do not ask the DHCP client to request and perhaps
# override that manual setting in this case. Exit code for both is 1.
# Note that this flag only controls whether we change the service state;
# even if the unit is disabled, we can change its configuration file based
# on data received (or not) from DHCP.
# TODO: Differentiate somehow if the user also wanted a specific NTP
# server vs. one received from DHCP?
NTP_SYSTEMD_NAME=""
{ NTP_SYSTEMD_ENABLED="`/bin/systemctl is-enabled ntpd 2>/dev/null`" && NTP_SYSTEMD_NAME="ntpd" ; } \
|| { NTP_SYSTEMD_ENABLED="`/bin/systemctl is-enabled ntp 2>/dev/null`" && NTP_SYSTEMD_NAME="ntp" ; } \
||  if [ $? != 1 -o -z "$NTP_SYSTEMD_ENABLED" ]; then
        NTP_SYSTEMD_ENABLED="unknown"
    fi

can_manipulate_ntpd() {
	if [ -z "$NTP_SYSTEMD_NAME" ] ; then
	    echo "$0: WARN: The NTP service unit name could not be determined so the DHCP hook will not manipulate it" >&2
	    return 1
	fi
	case "$NTP_SYSTEMD_ENABLED" in
	    mask*|disabl*)
	        echo "$0: WARN: The NTP service unit status is currently $NTP_SYSTEMD_ENABLED so the DHCP hook will not manipulate it" >&2
	        return 1
	        ;;
	    enabl*) return 0 ;;
	    *)
	        echo "$0: WARN: The NTP service unit status is currently '$NTP_SYSTEMD_ENABLED' which has no special handling, so the DHCP hook will try to manipulate it" >&2
	        return 0
	        ;;
	esac
}

hostname_setup() {
    case "$reason" in
        bound|renew|BOUND|RENEW|REBIND|REBOOT)
            ;;
        *)
            echo "$0: WARN: hostname_setup got an unexpected reason '$reason', proceeding anyway" >&2
            ;;
    esac

    # Pass the DHCP-suggested name (if any), it wold be applied if nothing
    # is set in /etc/hostname yet, and then saved into the file.
    interface="$interface" \
        fty-hostname-setup "$hostname" "true"
}

ntp_server_restart_do() (
	can_manipulate_ntpd || return $?

	invoke-rc.d "${NTP_SYSTEMD_NAME}" try-restart && \
	    echo "$0: INFO: NTP service restarted; waiting for it to pick up time (if not failed) so as to sync it onto hardware RTC" && \
	    sleep 60 && ntp_server_status && hwclock -w -u && \
	    echo "$0: INFO: Applied current OS clock value (`TZ=UTC date -u`) to HW clock (`TZ=UTC hwclock -r -u`); done with NTP restart"
)

ntp_server_restart() {
	can_manipulate_ntpd || return $?

	ntp_server_restart_do &
	sleep 5
	[ -d /proc/$! ] && echo "$0: INFO: NTP service restart process $! is still running, leaving it in the background" || wait $?
	# If that restarter is still running - let it run;
	# otherwise return its exit code (failed early)
}

ntp_server_status() {
	can_manipulate_ntpd || return $?

	invoke-rc.d "${NTP_SYSTEMD_NAME}" status
	# NOTE: successful return means the daemon is running, but
	# it does guarantee we've picked up time from any source
}


ntp_servers_setup_remove() {
	if [ ! -e "$NTP_DHCP_CONF" ]; then
		return
	fi
	rm -f "$NTP_DHCP_CONF"
	ntp_server_restart
}


ntp_servers_setup_add() {
	if [ -e $NTP_DHCP_CONF ] && [ "$new_ntp_servers" = "$old_ntp_servers" ]; then
		echo "Got no changes to apply to DHCP-announced NTP config"
		return
	fi

	if [ -z "$new_ntp_servers" ]; then
		echo "DHCP announced no NTP servers, falling back to static NTP configs..."
		ntp_servers_setup_remove
		return
	fi

	tmp=$(mktemp "$NTP_DHCP_CONF.XXXXXX") || return
	chmod --reference="$NTP_CONF" "$tmp"
	chown --reference="$NTP_CONF" "$tmp"

	echo "DHCP announced NTP servers '$new_ntp_servers' different from old NTP config '$old_ntp_servers', applying..."
	(
	  echo "# This file was copied from $NTP_CONF with the server options changed"
	  echo "# to reflect the information sent by the DHCP server.  Any changes made"
	  echo "# here will be lost at the next DHCP event.  Edit $NTP_CONF instead."
	  echo
	  echo "# NTP server entries received from DHCP server"
	  for server in $new_ntp_servers; do
		echo "server $server iburst"
	  done
	  echo
	  sed -r -e '/^ *(server|peer).*$/d' "$NTP_CONF"
	) >>"$tmp"

	mv "$tmp" "$NTP_DHCP_CONF"

	ntp_server_restart
}


ntp_servers_setup() {
	RES=1
	echo "[`awk '{print $1}' < /proc/uptime`] `date` [$$]: Starting $0 (NTP) for DHCP state $reason..."
	case "$reason" in
		bound|renew|BOUND|RENEW|REBIND|REBOOT)
			ntp_servers_setup_add
			RES=$?
			;;
		deconfig|leasefail|nak|EXPIRE|FAIL|RELEASE|STOP)
			ntp_servers_setup_remove
			RES=$?
			;;
	esac
	echo "[`awk '{print $1}' < /proc/uptime`] `date` [$$]: Completed $0 (NTP) for DHCP state $reason, exit code $RES"
	return $RES
}

[ -n "${domainname}" ] && domainname "${domainname}"

[ -z "${reason-}" -a -n "$1" ] && reason="$1"
[ -z "${new_ntp_servers-}" ] && new_ntp_servers="" && [ -n "${ntpsrv-}" ] && \
        new_ntp_servers="`for S in $ntpsrv; do echo "$S"; done | sort | uniq`"
if [ -z "${old_ntp_servers-}" ]; then
        old_ntp_servers=""
        if [ -s "$NTP_DHCP_CONF" ]; then
                old_ntp_servers="`cat "$NTP_DHCP_CONF" | egrep '^[ \t]*(server|peer)' | awk '{print $2}' | sort | uniq`"
        else
                old_ntp_servers="`cat "$NTP_CONF" | egrep '^[ \t]*(server|peer)' | awk '{print $2}' | sort | uniq`"
        fi
fi

if [ "`grep -lw debug /proc/cmdline`" ]; then
    set > /dev/console
fi

#exec >> /dev/console 2>&1
#set >&2
#set -x

hostname_setup

ntp_servers_setup
