#!/bin/sh

#
#   Copyright (c) 2020 Eaton
#
#   This file is part of the Eaton 42ity project.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    91-logrotate-fty.sh
#  \brief   Set up /etc/logrotate.d/* for this product (rsyslog handles its own)
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#
# Note: as services get obsoleted over time between different end-user releases,
# more copies of this script can appear since each copy is only executed once.
#

die() {
    echo "FATAL: $*" >&2
    exit 1
}

skip() {
    echo "SKIP: $*" >&2
    exit 0
}

mkdir -p /etc/logrotate.d.disabled

if [ -s /etc/logrotate.d/rsyslog ]; then
    echo "TWEAK existing /etc/logrotate.d/rsyslog"
    # Our rsyslog rotates messages and commands.log based on size by itself
    ( grep -v '/var/log/messages' < /etc/logrotate.d/rsyslog > /etc/logrotate.d/rsyslog.tmp ) \
    && ( cat /etc/logrotate.d/rsyslog.tmp > /etc/logrotate.d/rsyslog ; rm -f /etc/logrotate.d/rsyslog.tmp )
fi

if [ -s /etc/logrotate.d/mysql-server ] ; then
    echo "MOVE AWAY existing /etc/logrotate.d/mysql-server"
    mv -f /etc/logrotate.d/mysql-server /etc/logrotate.d.disabled/
fi

echo "CREATE NEW /etc/logrotate.d/fty-db-engine"
cat > /etc/logrotate.d/fty-db-engine << EOF
/var/log/mysql/*.log {
    daily
    size 100M
    rotate 7
    missingok
    # Directory access rights de-levation
    su mysql mysql
    # Recreate log file with these rights
    create 640 mysql bios-logread
    compress
    sharedscripts
    postrotate
        MYADMIN="/usr/bin/mysqladmin"
        test -x "\$MYADMIN" || exit 0

        if [ -s /etc/mysql/debian.cnf ] ; then
            MYADMIN="\$MYADMIN --defaults-file=/etc/mysql/debian.cnf"
        fi
        if [ -z "\`\$MYADMIN ping 2>/dev/null\`" ]; then
          if ps cax | grep -q mysqld; then
            exit 1
          fi
        else
          \$MYADMIN flush-logs
        fi
    endscript
}
EOF

echo "CREATE NEW /etc/logrotate.d/tntnet-bios"
cat > /etc/logrotate.d/tntnet-bios << EOF
# No need for /var/log/tntnet/www-audit.log - it is managed by log4cplus setting
/var/log/tntnet/tntnet.log /var/log/tntnet/access.log /var/log/tntnet/error.log {
    daily
    size 100M
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 0640 www-data bios-logread
    sharedscripts
### Note: At this time there is no known safe "reload" for tntnet daemon;
### common signals HUP and USR1 kill it completely (systemd restarts then)
#    postrotate
#        /usr/bin/timeout 30 /bin/systemctl reload --no-block tntnet@bios.service
#    endscript
}
EOF
