#!/bin/bash
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
#! \file    preinstallimage-bios.sh
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
nosoup4u
nosoup4u
EOF

groupadd -g 8003 bios-admin
groupadd -g 8002 bios-poweruser
groupadd -g 8001 bios-user
groupadd -g 8000 bios-guest
groupadd bios-infra
useradd -m bios -N -g bios-infra -G dialout -s /bin/bash
useradd -m admin -G sasl -N -g bios-admin -s /bin/bash
passwd admin <<EOF
admin
admin
EOF

# Workplace for the webserver and graph daemons
mkdir -p /var/lib/bios
chown -R www-data /var/lib/bios

# A few helper aliases
cat > /etc/profile.d/bios_aliases.sh << EOF
alias dbb='mysql -u root box_utf8 -t'
alias la='ls -la'
EOF

# BIOS PATH
cat > /etc/profile.d/bios_path.sh << EOF
export PATH="/usr/libexec/bios:\$PATH"
EOF

chmod a+rx /etc/profile.d/*

# BIOS configuration file
touch /etc/default/bios
chown www-data /etc/default/bios
chmod a+r /etc/default/bios
mkdir -p /etc/bios/nut/devices
chown -R bios:bios-infra /etc/bios
systemd-tmpfiles --create

# Setup BIOS lenses
mkdir -p /usr/share/bios/lenses
ln -sr /usr/share/augeas/lenses/dist/{build,ethers,interfaces,ntp,ntpd,pam,resolv,rx,sep,util,shellvars}.aug \
    /usr/share/bios/lenses

# Setup u-Boot
echo '/dev/mtd3 0x00000 0x40000 0x40000' > /etc/fw_env.config

# journald setup
sed -i 's|.*RuntimeMaxFileSize.*|RuntimeMaxFileSize=10M|' /etc/systemd/journald.conf
sed -i 's|.*Storage.*|Storage=volatile|'                  /etc/systemd/journald.conf

# Basic network setup
mkdir -p /etc/network

cat > /etc/network/interfaces <<EOF
auto lo
allow-hotplug eth0 LAN1 LAN2 LAN3
iface lo inet loopback
iface `file -b /bin/bash | sed -e 's|.*x86-64.*|eth0|' -e 's|.*ARM.*|LAN1|'` inet dhcp
iface LAN2 inet static
    address 192.168.1.10
    netmask 255.255.255.0
    gateway 192.168.1.1
iface LAN3 inet static
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

DEFAULT_IFPLUGD_INTERFACES="eth0 LAN1 LAN2 LAN3"
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
deb http://ftp.debian.org/debian jessie main contrib non-free
deb http://ftp.debian.org/debian jessie-updates main contrib non-free
deb http://security.debian.org   jessie/updates main contrib non-free
deb http://obs.roz53.lab.etn.com:82/Pool:/master/Debian_8.0 /
deb http://obs.roz53.lab.etn.com:82/Pool:/master:/proprietary/Debian_8.0 /
EOF

mkdir -p /etc/apt/preferences.d
cat > /etc/apt/preferences.d/bios <<EOF
Package: *
Pin: origin "obs.roz53.lab.etn.com"
Pin-Priority: 9999
EOF

cat << EOF | apt-key add -
-----BEGIN PGP PUBLIC KEY BLOCK-----
Version: GnuPG v2

mQGiBFafdbgRBACOJ2v+OPLFEIVSLDnibtuRUplIJFHHdHMn2PMPsYzxX67X/Liu
kk6nF+mNJsObLTHsPk99l3Qctt5Qn4MOspylVv/ieoiacs1qhcvVtpFv9V6rCjhn
ZBi5YgXdiXk5Vp9N2nSqkUKdE+ycBf3ks6gaE517SO6KAxZtG6I3v0ychwCgyqo6
xQh0xBAuan0bZCeD3QXMNGcD/3DPiw+1fluUv1j0SmhDwfqTkWI9yIDitDljx1AJ
vD/icREUVck907YCASVOgbM8hBKhABOOx22u6YtSVxMCAwnm6m9wwxvxILjjVajs
i8s44X6ng3ia2AzQgH2mwU2DOKFPTbcmkExCEwejH5Ef4k3DSfzaDcznmSDN4fNL
0VEcA/4iRR7kOt5p05Fm//Ob826MCD7BVsnikgVAQrks8omh/f/SgwNNwxLMziY0
JnJw0E9lK711FcxgZNnk4xxwHEnPjkVpFV18A+b+ldwhZ8I05Pw6Okr/f47MF9/O
pDlLPC6Va/c18v5hTIckRrBOh2nzPyJH7gKhTXCEFautYLOldbQ6cHJpdmF0ZSBP
QlMgKGtleSB3aXRob3V0IHBhc3NwaHJhc2UpIDxkZWZhdWx0a2V5QGxvY2Fsb2Jz
PohjBBMRAgAjBQJWn3W4AhsjBwsJCAcDAgEGFQgCCQoLBBYCAwECHgECF4AACgkQ
Myo8Ka8xTGhsnQCeMIzLSpdZG+zIaN4LtSeY/uS66+IAn0QOuM+EeK7ojKCkhTGY
Pcz3AHDquQENBFafdbgQBADzEHRnwTweK2cWT1pwoiNYbHkvksA1FicuUEhIEudd
tZE2VmbkvPnhFTzuC3vMq2znVITE68LifArovqPCk538jrs/g3Rz1e+540Ccq/QY
O2i5qJTQAD6TltNvnbR3juA0t9Pf8dp9OcpxD0sZeCt8qxcl/aEuAaEzXSFntmgI
rwADBQQAzDN3MN3YtaRhtJgzvYLjiIocejKBRadRKjudcIGEM+dltfQeYIwyZR1y
TItx3zoioTwRvJw6y51HPn2liyMRRAcql1IAqJv7Hbvg38aj9abQ3MDbvp1kzGFK
HABTa6I/DpREO2X35UnqJ91LYzRx26DC4PNHanGx4ZqgIRiLwyKISQQYEQIACQUC
Vp91uAIbDAAKCRAzKjwprzFMaD2ZAJ9uGAv1l4o1X88jpBT6wOUSX+3vHgCePkle
wscGBRaOr9hO4FqPjem8H6s=
=sSgW
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
RULES="`sed -n 's|.*pam_cracklib.so||p' /etc/pam.d/bios`"
[ "$IMGTYPE" = devel ] || sed -i "s|\\(.*pam_cracklib.so\\).*|\1$RULES|" /etc/pam.d/common-password

sed -i 's|\(secure_path="\)|\1/usr/libexec/bios:|' /etc/sudoers

mkdir -p /etc/sudoers.d
cp /usr/share/bios/examples/config/sudoers.d/bios_00_base /etc/sudoers.d
[ "$IMGTYPE" = devel ] && cp /usr/share/bios/examples/config/sudoers.d/bios_01_citest /etc/sudoers.d

mkdir -p /etc/security
cp /usr/share/bios/examples/config/security/* /etc/security
sed -i 's|START=no|START=yes|' /etc/default/saslauthd
systemctl enable saslauthd

mkdir -p /etc/update-rc3.d
cp /usr/share/bios/examples/config/update-rc3.d/* /etc/update-rc3.d
[ -n "$IMGTYPE" ] && \
    echo "IMGTYPE='$IMGTYPE'" > /etc/update-rc3.d/image-os-type.conf

# Enable mysql
systemctl enable mysql

# Enable ssh
echo "UseDNS no" >> /etc/ssh/sshd_config
rm /etc/ssh/*key*
mkdir -p /etc/systemd/system
sed 's|\[Service\]|[Service]\nExecStartPre=/usr/bin/ssh-keygen -A|' /lib*/systemd/system/ssh@.service > /etc/systemd/system/ssh@.service
sed -i 's|\[Unit\]|[Unit]\nConditionPathExists=/var/lib/bios/license\nConditionPathExists=/mnt/nand/overlay/etc/shadow|' /etc/systemd/system/ssh@.service
systemctl disable ssh.service
systemctl mask ssh.service
systemctl enable ssh.socket

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
if [ "`uname -m`" = x86_64 ]; then
    systemctl enable bios-fake-th
    systemctl disable agent-th
    systemctl disable lcd-boot-display
    systemctl disable lcd-net-display
    systemctl mask lcd-boot-display
    systemctl mask lcd-net-display
    systemctl disable bios-reset-button
    systemctl mask bios-reset-button
else
    systemctl disable bios-fake-th
    systemctl mask bios-fake-th
    systemctl enable lcd-boot-display
    systemctl enable lcd-net-display
    sed -i 's|PathChanged=/etc|PathChanged=/mnt/nand/overlay/etc|' /usr/lib/systemd/system/composite-metrics\@.path
fi
# Services not part of core
systemctl enable bios-agent-legacy-metrics
systemctl enable bios-agent-alert-generator

# Our tntnet unit
cat > /etc/systemd/system/tntnet@.service <<EOF
[Unit]
Description=Tntnet web server using /etc/tntnet/%I.xml
After=network.target bios-db-init.service
Requires=bios-db-init.service
PartOf=bios.target

[Service]
Type=simple
EnvironmentFile=-/etc/default/bios
EnvironmentFile=-/etc/sysconfig/bios
EnvironmentFile=-/etc/default/bios__%n.conf
EnvironmentFile=-/etc/sysconfig/bios__%n.conf
EnvironmentFile=-/etc/default/bios-db-rw
PrivateTmp=true
ExecStartPre=/usr/share/bios/scripts/ssl-create.sh
ExecStartPre=/usr/share/bios/scripts/xml-cat.sh /etc/tntnet/%i.d /etc/tntnet/%i.xml
ExecStart=/usr/bin/tntnet -c /etc/tntnet/%i.xml
Restart=on-failure

[Install]
WantedBy=bios.target
EOF
cat > /usr/share/bios/scripts/xml-cat.sh << EOF
#!/bin/sh
cat "\$1"/*.xml > "\$2"
EOF
chmod a+rx /usr/share/bios/scripts/xml-cat.sh
rm -f /etc/init.d/tntnet

# Enable REST API via tntnet
mkdir -p /etc/tntnet/bios.d
cp /usr/share/bios/examples/tntnet.xml.* /etc/tntnet/bios.xml
mkdir -p /usr/share/core-0.1/web/static
sed -i 's|<!--.*<user>.*|<user>www-data</user>|' /etc/tntnet/bios.xml
sed -i 's|<!--.*<group>.*|<group>sasl</group>|' /etc/tntnet/bios.xml
sed -i 's|.*<daemon>.*|<daemon>0</daemon>|' /etc/tntnet/bios.xml
sed -i 's|\(.*\)<dir>.*|\1<dir>/usr/share/bios-web/</dir>|' /etc/tntnet/bios.xml
sed -n '1,/<mappings>/ p' /etc/tntnet/bios.xml  > /etc/tntnet/bios.d/00_start.xml
sed -n '/<\/mappings>/,$ p' /etc/tntnet/bios.xml > /etc/tntnet/bios.d/99_end.xml
sed '/<mappings>/,/<\/mappings>/!d; /mappings/ d' /etc/tntnet/bios.xml > /etc/tntnet/bios.d/20_core.xml
sed '1,/<!-- Here starts the real API -->/!d; /<!-- Here starts the real API -->/ d' /etc/tntnet/bios.d/20_core.xml > /etc/tntnet/bios.d/10_common_basics.xml
sed '/<!-- Here starts the real API -->/,$!d; /<!-- Here starts the real API -->/ d' /etc/tntnet/bios.d/20_core.xml > /etc/tntnet/bios.d/50_main_api.xml
rm -f /etc/tntnet/bios.d/20_core.xml
systemctl enable tntnet@bios
systemctl enable tntnet@bios

# Disable logind
systemctl disable systemd-logind
find / -name systemd-logind.service -delete

# Disable expensive debug logging by default on non-devel images
mkdir -p /etc/default/
if [ "$IMGTYPE" = devel ] ; then
    echo "BIOS_LOG_LEVEL=LOG_DEBUG" > /etc/default/bios
else
    echo "BIOS_LOG_LEVEL=LOG_INFO" > /etc/default/bios
    sed -i 's|.*MaxLevelStore.*|MaxLevelStore=info|'                  /etc/systemd/journald.conf
fi
# set path to our libexec directory
echo "PATH=/usr/libexec/bios:/bin:/usr/bin:/sbin:/usr/sbin" >>/etc/default/bios

# Setup some busybox commands
for i in vi tftp wget; do
   ln -s busybox /bin/$i
done

# Simplify ntp.conf
augtool -S -I/usr/share/bios/lenses << EOF
rm /files/etc/ntp.conf/server
set /files/etc/ntp.conf/server[1] pool.ntp.org
save
EOF

# BIOS emulator script which can fake some of the curl behaviour with wget
[ ! -x /usr/bin/curl ] && [ -x /usr/share/bios/scripts/curlbbwget.sh ] && \
    install -m 0755 /usr/share/bios/scripts/curlbbwget.sh /usr/bin/curl


#########################################################################
# Setup zabbix
# TODO: revise the list of BIOS services here
if [ -f /usr/bin/zabbix_agent ]; then
for i in mysql tntnet@bios malamute \
    bios-db bios-server-agent bios-agent-inventory bios-agent-nut bios-driver-netmon \
    nut-driver nut-monitor systemd-journald \
; do
   find /lib /usr -name "$i".service | while read file; do
       sed -i 's|\(\[Service\]\)|\1\nMemoryAccounting=yes\nCPUAccounting=yes\nBlockIOAccounting=yes|' "$file"
   done
done

sed -i 's|127.0.0.1|greyhound.roz53.lab.etn.com|' /etc/zabbix/zabbix_agentd.conf
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
fi
# End of setup of zabbix
#########################################################################

# Set lang
echo 'export LANG="C"' > /etc/profile.d/lang.sh
for V in LANG LANGUAGE LC_ALL ; do echo "$V="'"C"'; done > /etc/default/locale

# logout from /bin/bash after 600s/10m of inactivity
echo 'export TMOUT=600' > /etc/profile.d/tmout.sh

# Help ifup and ifplugd do the right job
install -m 0755 /usr/share/bios/scripts/ethtool-static-nolink /etc/network/if-pre-up.d
install -m 0755 /usr/share/bios/scripts/ifupdown-force /etc/ifplugd/action.d/ifupdown-force
install -m 0755 /usr/share/bios/scripts/udhcpc-override.sh /usr/local/sbin/udhcpc
echo '[ -s /usr/share/bios/scripts/udhcpc-ntp.sh ] && . /usr/share/bios/scripts/udhcpc-ntp.sh' >> /etc/udhcpc/default.script

#########################################################################
# install iptables filtering

## startup script
cat >/etc/network/if-pre-up.d/iptables-up <<[eof]
#!/bin/sh
test -r /etc/default/iptables && iptables-restore < /etc/default/iptables
test -r /etc/default/ip6tables && ip6tables-restore < /etc/default/ip6tables
[eof]
chmod 755 /etc/network/if-pre-up.d/iptables-up

## ipv4 default table
cat > /etc/default/iptables <<[eof]
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A INPUT -i lo -j ACCEPT
-A INPUT -d 127.0.0.0/8 ! -i lo -j REJECT --reject-with icmp-port-unreachable
-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
-A INPUT -p tcp -m tcp --dport 80 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 443 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 22 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 4222 -j ACCEPT
-A INPUT -p icmp -j ACCEPT
-A INPUT -j REJECT --reject-with icmp-port-unreachable
-A FORWARD -j REJECT --reject-with icmp-port-unreachable
-A OUTPUT -j ACCEPT
COMMIT
[eof]

## ipv6 default table
cat > /etc/default/ip6tables <<[eof]
*filter
:INPUT ACCEPT [0:0]
:FORWARD ACCEPT [0:0]
:OUTPUT ACCEPT [0:0]
-A INPUT -i lo -j ACCEPT
-A INPUT -d ::1/128 ! -i lo -j REJECT --reject-with icmp6-port-unreachable
-A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT
-A INPUT -p tcp -m tcp --dport 80 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 443 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 22 -j ACCEPT
-A INPUT -p tcp -m tcp --dport 4222 -j ACCEPT
-A INPUT -p icmpv6 -j ACCEPT
-A INPUT -j REJECT --reject-with icmp6-port-unreachable
-A FORWARD -j REJECT --reject-with icmp6-port-unreachable
-A OUTPUT -j ACCEPT
COMMIT
[eof]

# install iptables filtering end
#########################################################################

# More space saving
SPACERM="rm -rf"
$SPACERM /usr/share/nmap/nmap-os-db /usr/bin/{aria_read_log,aria_dump_log,aria_ftdump,replace,resolveip,myisamlog,myisam_ftdump}
case "$SPACERM" in
    rm|rm\ *) # Replace cleaned-up stuff
        install -m 0755 /usr/share/bios/scripts/resolveip.sh /usr/bin/resolveip
        ;;
esac
for i in /usr/share/mysql/* /usr/share/locale /usr/share/bios/{develop,obs}; do
   [ -f "$i" ] || \
   [ "$i" = /usr/share/mysql/charsets ] || \
   [ "$i" = /usr/share/mysql/english ] || \
   $SPACERM "$i" || true
done

# Show the package list
dpkg --get-selections
dpkg-query -Wf '${Installed-Size}\t${Package}\n' | sort -n

# Create the CSV Legal packages manifest, to display from the Web UI
# copyright files path are adapted to Web UI display!
CSV_FILE_PATH="/usr/share/doc/ipc-packages.csv"
rm -f ${CSV_FILE_PATH}
touch ${CSV_FILE_PATH}
dpkg-query -W -f='${db:Status-Abbrev};${source:Package};${Version};${binary:Package}\n' | grep '^i' | cut -d';' -f2,3,4 | sort -u > ./pkg-list.log
while IFS=';' read SOURCE_PKG PKG_VERSION CURRENT_PKG
do
   CURRENT_PKG="`echo ${CURRENT_PKG} | cut -d':' -f1`"
   grep -q "${SOURCE_PKG};${PKG_VERSION};" "${CSV_FILE_PATH}" || \
   if [ $? -eq 1 ]; then
      echo "${SOURCE_PKG};${PKG_VERSION};/legal/${CURRENT_PKG}/copyright" 2>/dev/null >> "${CSV_FILE_PATH}"
      [ ! -f "/usr/share/doc/${CURRENT_PKG}/copyright" ] && echo "Missing ${CURRENT_PKG}/copyright file!"
   fi
done < ./pkg-list.log
rm -f ./pkg-list.log

# Prepare the ccache (for development image type)
case "$IMGTYPE" in
    devel)
        [ -x /usr/sbin/update-ccache-symlinks ] && \
		/usr/sbin/update-ccache-symlinks || true
        # If this image ends up on an RC3, avoid polluting NAND with ccache
        mkdir -p /home/admin/.ccache
        chown -R admin /home/admin/.ccache
        rm -rf /root/.ccache
        ln -s /home/admin/.ccache /root/.ccache
        echo "export PATH=\"/usr/lib/ccache:/usr/lib64/ccache:\$PATH\"" > /etc/profile.d/ccache.sh
        ;;
esac

# Prepate the source-code details excerpt, if available
[ -s "/usr/share/bios/.git_details" ] && \
    grep ESCAPE "/usr/share/bios/.git_details" > /usr/share/bios-web/git_details.txt || \
    echo "WARNING: Do not have /usr/share/bios/.git_details"

# Timestamp the end of OS image generation
LANG=C date -u > /usr/share/bios-web/image-version.txt || \
    echo "WARNING: Could not record OBS image-building timestamp"

# Get rid of static qemu binaries needed for crossinstallation
# TODO: Integrate this better into build-recipe-preinstallimage/init_buildsystem
rm -f /usr/bin/qemu*

echo "INFO: successfully reached the end of script: $0 $@"
