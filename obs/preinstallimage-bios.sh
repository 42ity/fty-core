#!/bin/sh
#
#   Copyright (c) 2014-2015 Eaton
#
#   This file is part of the Eaton $BIOS project.
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
#! \file    restapi-request.sh
#  \brief   Script to generate the expected directory structure and configuration files
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Script to generate the expected directory structure
#   and configuration files "baked into" the read-only OS images that
#   are prepared by OBS for dev/test X86 containers as well as the
#   ultimate RC3 environments. Any changes to the files "hardcoded"
#   here can be applied by the deployed systems either as overlay
#   filesystem or unpacked tarballs.
#
#   This code used to be part of a spec-like file hidden in the OBS
#   job setup - now the code part can be tracked in common Git sources.
#   Note there are also spec-headers with different BuildRequires and
#   BuildIgnore sets of Debian packages for different OS image types,
#   which are still parts of the "hidden" OBS _preinstallimage recipes.
#
#   Also note that some parts of the setup here are geared towards
#   internal dev-test deployments in Eaton network, at the moment.
#
#   This script is executed in the chroot'ed filesystem tree prepared
#   by OBS installation of packages, including the BIOS core package.
#   It is called as the "%build" recipe implementation from the OBS
#   specfile, with an IMGTYPE envvar pre-set to "devel" or "deploy".

echo "INFO: Executing $0 $*"
echo "    IMGTYPE='$IMGTYPE'"

# Protect against errors... such as maybe running on a dev workstation
set -e

# Setup core dumps
if true; then
    mkdir -p /var/crash
    chmod 1777 /var/crash
    ( echo 'kernel.core_pattern = /var/crash/%t-%e-%s.core'
      echo 'fs.suid_dumpable = 2' \
    ) > /etc/sysctl.d/10-core.conf
    sed -i 's|.*DefaultLimitCORE=.*|DefaultLimitCORE=infinity|' /etc/systemd/system.conf
    sed -i 's|.*DumpCore=.*|DumpCore=yes|' /etc/systemd/system.conf
fi

# Create user and set root password
passwd <<EOF
@PASSWORD@
@PASSWORD@
EOF

useradd -m bios -G sasl -s /bin/bash
passwd bios <<EOF
@PASSWORD@
@PASSWORD@
EOF

# Workplace for the webserver and graph daemons
mkdir -p /var/lib/bios
chown -R bios:bios /var/lib/bios

# A few helper aliases
cat > /etc/profile.d/bios_aliases.sh << EOF
alias dbb='mysql -u root box_utf8 -t'
alias la='ls -la'
EOF

# Setup u-Boot
echo '/dev/mtd3 0x00000 0x40000 0x40000' > /etc/fw_env.config

# journald setup
sed -i 's|.*RuntimeMaxFileSize.*|RuntimeMaxFileSize=10M|' /etc/systemd/journald.conf
sed -i 's|.*Storage.*|Storage=volatile|'                  /etc/systemd/journald.conf

# Basic network setup
mkdir -p /etc/network

cat > /etc/network/interfaces <<EOF
auto lo
allow-hotplug eth0 eth1 eth2
iface lo inet loopback
iface eth0 inet dhcp
iface eth1 inet static
    address 192.168.1.10
    netmask 255.255.255.0
    gateway 192.168.1.1
iface eth2 inet static
    address 192.168.2.10
    netmask 255.255.255.0
    gateway 192.168.2.1
EOF

cat > /etc/resolv.conf <<EOF
nameserver 8.8.8.8
nameserver 8.8.4.4
EOF

cat > /etc/hosts <<EOF
127.0.0.1 localhost bios
EOF

DEFAULT_IFPLUGD_INTERFACES="eth0 eth1 eth2"
mkdir -p /etc/default
[ -s "/etc/default/networking" ] && \
    sed -e 's,^[ \t\#]*\(EXCLUDE_INTERFACES=\)$,\1"'"$DEFAULT_IFPLUGD_INTERFACES"'",' -i /etc/default/networking \
    || echo 'EXCLUDE_INTERFACES="'"$DEFAULT_IFPLUGD_INTERFACES"'"' >> /etc/default/networking
cat > /etc/default/ifplugd <<EOF
INTERFACES="$DEFAULT_IFPLUGD_INTERFACES"
HOTPLUG_INTERFACES=""
ARGS="-q -f -u0 -d10 -w -I"
SUSPEND_ACTION="stop"
EOF


# Setup APT package sources
mkdir -p /etc/apt/sources.list.d
cat > /etc/apt/sources.list.d/debian.list <<EOF
deb http://ftp.debian.org/debian testing main contrib non-free
deb http://ftp.debian.org/debian jessie-updates main contrib non-free
deb http://security.debian.org   jessie/updates main contrib non-free
deb http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0 /
EOF

mkdir -p /etc/apt/preferences.d
cat > /etc/apt/preferences.d/bios <<EOF
Package: *
Pin: origin "obs.roz.lab.etn.com"
Pin-Priority: 9999
EOF

cat << EOF | apt-key add -
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2.0.22 (GNU/Linux)

mQENBFQ1HhIBCADc78sdWj+c8ag3j1LZRfX3F/BVzAfsUvo1kDHcexMnts7gDKdA
63YXzD8RkDwYDq1pmuz+GEUNl0WinYiCkNgBgM9YKyBQdGZbH972rqWz/ZDxDe/2
iBJ+PvtLfJC1y2yx22LFWOseBg4bpQ3h9qYoK0rILfRXxI33Lnt/RMxlOPWNRiJH
9DJAPyOaglN+spEdMjcsHRTtqKMsS4ZaR2FqgLq3Z6Zatx7OSgBOD2xBke8bWM0N
bwKWuGH+ttGbMwKzWPrf9/lFknHGQiSBd0422F2c0d52m5OwtI/M7WMYo5hHPJgj
W9UOG+p+l0lbuTFsseVLNTH1/4YtC83RYarXABEBAAG0OVBvb2w6bWFzdGVyIE9C
UyBQcm9qZWN0IDxQb29sOm1hc3RlckBvYnMucm96LmxhYi5ldG4uY29tPokBPwQT
AQIAKQUCVDUeEgIbAwUJBB6wAAcLCQgHAwIBBhUIAgkKCwQWAgMBAh4BAheAAAoJ
EC5px1OX88+7GnYH/ieONgY2PNeWAHkHnCqyJGN+dXfX3owsihS8+/doI32cN4nK
wLIbheFy37edEfDbDV3WIU23EHSnpY52e43FU8OKKBXnuWdrFxUn/Phttq6Tr5Zc
W3Bl13EQx7BI4tINVXwXYFSdikXPQTNazQAD5Dks0Hdqt3wsSj6ht6t6JIe8UtRJ
VmDd3rARFdpsDMwEPFbxu2RZA8QUzbbvaRyzJ+YjlZgYbMiMo2wUCHdXfWcz7XZj
nR3ncHfgisSQ66i6Dzjw2T0gbAYM8pY15P/3WjpnrBe+W7n48G2wvjwIgkVPWpSb
JEKEbP0i/lTKvtz24kkEmYbbHtBDXPWneSjWfn+IRgQTEQIABgUCVDUeEgAKCRB5
twKQVKv3XkLwAKCCTcBHvIRT7hKkToCOIHOegWHiXwCeK4D+343Wjqv5k3CMnb5k
H5frBJ4=
=WLxM
-----END PGP PUBLIC KEY BLOCK-----
EOF

# Uninstall various packages that are not needed
for i in sysvinit ncurses-common libicu52 lsb-release; do
    case "$IMGTYPE" in
        devel)
            echo dpkg -P --force-all $i
            ;;
        deploy|*)
            dpkg -P --force-all $i
            ;;
    esac
done

# Setup bios security
mkdir -p /etc/pam.d
cp /usr/share/bios/examples/config/pam.d/* /etc/pam.d

mkdir -p /etc/sudoers.d
cp /usr/share/bios/examples/config/sudoers.d/bios_00_base /etc/sudoers.d

mkdir -p /etc/security
cp /usr/share/bios/examples/config/security/* /etc/security
sed -i 's|START=no|START=yes|' /etc/default/saslauthd
systemctl enable saslauthd

# Enable mysql
systemctl enable mysql

# Enable ssh
echo "UseDNS no" >> /etc/ssh/sshd_config
rm /etc/ssh/*key*
sed -i 's|\[Service\]|[Service]\nExecStartPre=/usr/bin/ssh-keygen -A|' /lib*/systemd/system/ssh.service
systemctl enable ssh

# Workaround nutscanner's ldopen
ln -sr /usr/lib/*/libnetsnmp.so.*.* /usr/lib/libnetsnmp.so
ln -sr /lib/*/libupsclient.so.*.*   /lib/libupsclient.so
ln -sr /lib/*/libusb-*.so.*.*       /lib/libusb.so
ln -sr /usr/lib/libneon.so.*.*      /usr/lib/libneon.so

# Enable malamute with BIOS configuration
mkdir -p /etc/malamute
cp /usr/share/bios/examples/config/malamute/malamute.cfg /etc/malamute
systemctl enable malamute

# Enable BIOS services (distributed as a systemd preset file)
systemctl preset-all

# Fix tntnet unit
cat > /usr/lib/systemd/system/tntnet@.service <<EOF
[Unit]
Description=Tntnet web server using /etc/tntnet/%I.xml
After=network.target bios-db-init.service
Requires=bios-db-init.service

[Service]
Type=simple
PrivateTmp=true
ExecStart=/usr/bin/tntnet -c /etc/tntnet/%i.xml
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF
rm -f /etc/init.d/tntnet

# Enable REST API via tntnet
cp /usr/share/bios/examples/tntnet.xml.* /etc/tntnet/bios.xml
mkdir -p /usr/share/core-0.1/web/static
sed -i 's|<!--.*<user>.*|<user>bios</user>|' /etc/tntnet/bios.xml
sed -i 's|\(.*\)<port>.*|\1<port>80</port>|' /etc/tntnet/bios.xml
sed -i 's|<!--.*<group>.*|<group>sasl</group>|' /etc/tntnet/bios.xml
sed -i 's|.*<daemon>.*|<daemon>0</daemon>|' /etc/tntnet/bios.xml
sed -i 's|\(.*\)<dir>.*|\1<dir>/usr/share/bios-web/</dir>|' /etc/tntnet/bios.xml
systemctl enable tntnet@bios

# Disable logind
systemctl disable systemd-logind
find / -name systemd-logind.service -delete

# Setup some busybox commands
for i in vi tftp wget; do
   ln -s busybox /bin/$i
done

# BIOS emulator script which can fake some of the curl behaviour with wget
[ ! -x /usr/bin/curl ] && [ -x /usr/share/bios/scripts/curlbbwget.sh ] && \
    install -m 0755 /usr/share/bios/scripts/curlbbwget.sh /usr/bin/curl


#########################################################################
# Setup zabbix
# TODO: revise the list of BIOS services here
for i in mysql tntnet@bios malamute \
    bios-db bios-server-agent bios-agent-inventory bios-agent-nut bios-driver-netmon \
    nut-driver nut-monitor systemd-journald \
; do
   find /lib /usr -name "$i".service | while read file; do
       sed -i 's|\(\[Service\]\)|\1\nMemoryAccounting=yes\nCPUAccounting=yes\nBlockIOAccounting=yes|' "$file"
   done
done

sed -i 's|127.0.0.1|greyhound.roz.lab.etn.com|' /etc/zabbix/zabbix_agentd.conf
sed -i 's|^Hostname|#\ Hostname|' /etc/zabbix/zabbix_agentd.conf
# Our network sucks, use longer timeouts
sed -i 's|#\ Timeout.*|Timeout=15|' /etc/zabbix/zabbix_agentd.conf
systemctl enable zabbix-agent
sed -i 's|\(chown -R.*\)|\1\nmkdir -p /var/log/zabbix-agent\nchown zabbix:zabbix /var/log/zabbix-agent|' /etc/init.d/zabbix-agent

cat > /etc/zabbix/zabbix_agentd.conf.d/mysql.conf << EOF
UserParameter=mysql.status[*],echo "show global status where Variable_name='\$1';" | mysql -N -u root | awk '{print \$\$2}'
UserParameter=mysql.ping,mysqladmin -u root ping | grep -c alive
UserParameter=mysql.version,mysql -V
EOF

cat > /etc/zabbix/zabbix_agentd.conf.d/systemd.conf << EOF
UnsafeUserParameters=1
UserParameter=system.systemd.service.cpushares[*],find /sys/fs/cgroup/cpu*         -name "\$1.service" -exec cat \\{\\}/cpuacct.usage \\;
UserParameter=system.systemd.service.memory[*],find /sys/fs/cgroup/memory          -name "\$1.service" -exec cat \\{\\}/memory.usage_in_bytes \\;
UserParameter=system.systemd.service.blkio[*],expr 0 + \`find /sys/fs/cgroup/blkio -name "\$1.service" -exec sed -n "s|.*\$2\ | + |p" \\{\\}/blkio.throttle.io_service_bytes \\;\`
UserParameter=system.systemd.service.processes[*],find /sys/fs/cgroup/systemd      -name "\$1.service" -exec cat \\{\\}/tasks \\; | wc -l
EOF

cat > /etc/zabbix/zabbix_agentd.conf.d/iostat.conf << EOF
UserParameter=custom.vfs.dev.discovery,/etc/zabbix/scripts/queryDisks.sh
                              
# reads completed successfully                                                                       
UserParameter=custom.vfs.dev.read.ops[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$4}'
# sectors read                                                                                           
UserParameter=custom.vfs.dev.read.sectors[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$6}'
# time spent reading (ms)                                                                           
UserParameter=custom.vfs.dev.read.ms[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$7}'
# writes completed                                                                                    
UserParameter=custom.vfs.dev.write.ops[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$8}'
# sectors written                                                                                          
UserParameter=custom.vfs.dev.write.sectors[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$10}'
# time spent writing (ms)                                                                             
UserParameter=custom.vfs.dev.write.ms[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$11}'
# I/Os currently in progress                                                                           
UserParameter=custom.vfs.dev.io.active[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$12}'
# time spent doing I/Os (ms)                                                                       
UserParameter=custom.vfs.dev.io.ms[*],cat /proc/diskstats | egrep \$1 | head -1 | awk '{print \$\$13}'
EOF
mkdir -p /etc/zabbix/scripts/
cat > /etc/zabbix/scripts/queryDisks.sh << EOF
#!/bin/sh
echo '{
        "data":[
                { "{#DISK}":"mmcblk0" },
                { "{#DISK}":"ubiblock0_0" }
        ]
}'
EOF
chmod a+rx /etc/zabbix/scripts/queryDisks.sh
# End of setup of zabbix
#########################################################################

# Set lang
echo 'export LANG="C"' > /etc/profile.d/lang.sh
for V in LANG LANGUAGE LC_ALL ; do echo "$V="'"C"'; done > /etc/default/locale

# Help ifup and ifplugd do the right job
install -m 0755 /usr/share/bios/scripts/ethtool-static-nolink /etc/network/if-pre-up.d
install -m 0755 /usr/share/bios/scripts/ifupdown-force /etc/ifplugd/action.d/ifupdown-force
install -m 0755 /usr/share/bios/scripts/udhcpc-override.sh /usr/local/sbin/udhcpc
echo '[ -s /usr/share/bios/scripts/udhcpc-ntp.sh ] && . /usr/share/bios/scripts/udhcpc-ntp.sh' >> /etc/udhcpc/default.script

# More space saving
SPACERM="rm -rf"
$SPACERM /usr/share/nmap/nmap-os-db /usr/bin/{aria_read_log,aria_dump_log,aria_ftdump,replace,resolveip,myisamlog,myisam_ftdump}
case "$SPACERM" in
    rm|rm\ *) # Replace cleaned-up stuff
        install -m 0755 /usr/share/bios/scripts/resolveip.sh /usr/bin/resolveip
        ;;
esac
for i in /usr/share/mysql/* /usr/share/locale /usr/share/bios/{docker,examples,develop,obs}; do
   [ -f "$i" ] || \
   [ "$i" = /usr/share/mysql/charsets ] || \
   [ "$i" = /usr/share/mysql/english ] || \
   $SPACERM "$i"
done

# Show the package list
dpkg --get-selections
dpkg-query -Wf '${Installed-Size}\t${Package}\n' | sort -n

# Get rid of static qemu binaries needed for crossinstallation
# TODO: Integrate this better into build-recipe-preinstallimage/init_buildsystem
rm -f /usr/bin/qemu*

# Prepare the ccache (for development image type)
case "$IMGTYPE" in
    devel)
        /usr/sbin/update-ccache-symlinks
        # If this image ends up on an RC3, avoid polluting NAND with ccache
        mkdir -p /home/bios/.ccache
        chown -R bios:bios /home/bios/.ccache
        rm -rf /root/.ccache
        ln -s /home/bios/.ccache /root/.ccache
        echo "export PATH=\"/usr/lib/ccache:/usr/lib64/ccache:\$PATH\"" > /etc/profile.d/ccache.sh
        ;;
esac

# Prepate the source-code details excerpt, if available
[ -s "/usr/share/bios/.git_details" ] && \
    /bin/grep ESCAPE "/usr/share/bios/.git_details" > /usr/share/bios-web/git_details.txt || \
    echo "WARNING: Do not have /usr/share/bios/.git_details"

# Timestamp the end of OS image generation
[ -x /bin/date ] && \
    LANG=C /bin/date -u > /usr/share/bios-web/image-version.txt || \
    echo "WARNING: Could not record OBS image-building timestamp"

echo "INFO: successfully reached the end of script: $0 $@"
