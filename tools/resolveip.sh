#!/bin/sh
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
#! \file    resolveip.sh
#  \brief   Replacement for the huge MySQL resolveip
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Replacement for the huge MySQL resolveip which is required
#           for hands-off creation of a default database

RES=0
SILENT=0
while [ $# -gt 0 ]; do
    REQT=""
    case "$1" in
	-s|--silent|--silent=[Tt][Rr][Uu][Ee])
		SILENT=1 ;;
	-V|--version)
		echo "resolveip Ver 1.0, for 42ity Project (`uname -m`)"
		exit 0
		;;
	-*)	;;	### Ignore, skip cycle
	*:*)	REQT="IPv6" ;;
	[0123456789]*.[0123456789]*.[0123456789]*.*[0123456789])
		REQT="IPv4" ;;
	*)	REQT="NAME" ;;
    esac

    if [ -n "$REQT" ]; then
	OUT="`getent hosts "$1"`"
	RESGE=$?
	[ $RESGE != 0 -o -z "$OUT" ] && RES=$RESGE

	echo "$OUT" | while read _IP _HN1 _HNOTHER; do
    	    if [ $? != 0 -o -z "$_IP" ]; then
                case "$REQT" in
                IP*)	echo "resolveip: Unable to find hostname for '$1'" >&2
                        RES=2 ;;
                *)	echo "resolveip: Unable to find hostid for '$1': host not found" >&2
                        RES=2 ;;
                esac
    	    else
                case "$REQT" in
                IP*)	[ "$SILENT" = 1 ] && echo "$_HN1" || \
			echo "Host name of $1 is $_HN1" ;;
                *)	[ "$SILENT" = 1 ] && echo "$_IP" || \
			echo "IP address of $1 is $_IP" ;;
                esac
    	    fi
	done
    fi

    shift
done

exit $RES
