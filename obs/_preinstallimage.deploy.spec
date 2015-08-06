#!BuildIgnore: gcc python make perl autoconf automake binutils
#!BuildIgnore: python2.7-minimal build-essential cpp cpp-4.6 dpkg-dev
#!BuildIgnore: perl perl-base python-minimal python2.7 python gcc-4.7-base
#!BuildIgnore: debhelper debianutils emdebian-grip emdebian-tdeb lintian
#!BuildIgnore: devscripts hardening-includes gcc-4.9-base vim vim-common vim-runtime
#
#!BuildIgnore: libclass-isa-perl
#!BuildIgnore: liblocale-gettext-perl
#!BuildIgnore: libapt-pkg-perl
#!BuildIgnore: libarchive-zip-perl
#!BuildIgnore: libdigest-hmac-perl
#!BuildIgnore: libencode-locale-perl
#!BuildIgnore: liberror-perl
#!BuildIgnore: libexporter-lite-perl
#!BuildIgnore: libhtml-tagset-perl
#!BuildIgnore: libio-string-perl
#!BuildIgnore: libio-stringy-perl
#!BuildIgnore: liblwp-mediatypes-perl
#!BuildIgnore: libnet-domain-tld-perl
#!BuildIgnore: libnet-http-perl
#!BuildIgnore: libnet-ip-perl
#!BuildIgnore: libtimedate-perl
#!BuildIgnore: libyaml-perl
#!BuildIgnore: libclone-perl
#!BuildIgnore: libio-pty-perl
#!BuildIgnore: libsub-name-perl
#!BuildIgnore: libclass-accessor-perl
#!BuildIgnore: libhttp-date-perl
#!BuildIgnore: libipc-run-perl
#!BuildIgnore: libparse-debian-packages-perl
#!BuildIgnore: libswitch-perl
#!BuildIgnore: liburi-perl
#!BuildIgnore: libdpkg-perl
#!BuildIgnore: libmailtools-perl
#!BuildIgnore: libnet-ssleay-perl
#!BuildIgnore: libnet-dns-perl
#!BuildIgnore: libwww-robotrules-perl
#!BuildIgnore: libio-socket-ssl-perl
#!BuildIgnore: libhtml-parser-perl
#!BuildIgnore: libparse-debianchangelog-perl
#!BuildIgnore: libemail-valid-perl
#!BuildIgnore: libhtml-tree-perl
#!BuildIgnore: libhttp-message-perl
#!BuildIgnore: libhttp-negotiate-perl
#!BuildIgnore: libhttp-cookies-perl
#!BuildIgnore: libwww-perl
#!BuildIgnore: liblwp-protocol-https-perl
#!BuildIgnore: libparse-debcontrol-perl
#!BuildIgnore: lsb-release

#!SimpleImage
Name: deploy-image
BuildRequires: iproute iptables udhcpc openssh-server bash ifupdown udev tntnet ntp watchdog
BuildRequires: iputils-ping apt udev mtd-utils systemd core sudo nut nut-snmp nut-xml bios-web
BuildRequires: tntdb-mysql4 libtntdb4 systemd-sysv malamute mariadb-server-10.0 sasl2-bin
BuildRequires: mime-support ncurses-base libgdbm3 zabbix-agent msmtp u-boot-tools ifplugd augeas-tools augeas-lenses
BuildRequires: libpam-cracklib libcrack2 cracklib-runtime wamerican

%build
# Setup core dumps
if true; then
	    mkdir -p /var/crash
	    chmod 1777 /var/crash
	    ( echo 'kernel.core_pattern = /var/crash/%t-%e-%s.core' ; echo 'fs.suid_dumpable = 2' ) > /etc/sysctl.d/10-core.conf
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

# Setup u-Boot
echo '/dev/mtd3 0x00000 0x40000 0x40000' > /etc/fw_env.config

# journald setup
sed -i 's|.*RuntimeMaxFileSize.*|RuntimeMaxFileSize=10M|' /etc/systemd/journald.conf
sed -i 's|.*Storage.*|Storage=volatile|'                  /etc/systemd/journald.conf

# Basic network setup
mkdir -p /etc/network
cat > /etc/network/interfaces <<EOF
auto lo eth0 eth1 eth2
allow-hotplug eth0 eth1 eth2
iface lo inet loopback
iface eth0 inet dhcp
iface eth1 inet static
    address 192.168.1.1
    netmask 255.255.255.0
iface eth2 inet manual
EOF
cat > /etc/resolv.conf <<EOF
nameserver 8.8.8.8
nameserver 8.8.4.4
EOF
cat > /etc/hosts <<EOF
127.0.0.1 localhost bios
EOF
mkdir -p /etc/default
cat > /etc/default/ifplugd <<EOF
INTERFACES="eth0 eth1 eth2"
HOTPLUG_INTERFACES=""
ARGS="-q -f -u0 -d10 -w -I"
SUSPEND_ACTION="stop"
EOF

# Setup sources
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
   dpkg -P --force-all $i
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

# Workaround nutscanners ldopen
ln -sr /usr/lib/*/libnetsnmp.so.*.* /usr/lib/libnetsnmp.so
ln -sr /lib/*/libupsclient.so.*.*   /lib/libupsclient.so
ln -sr /lib/*/libusb-*.so.*.*       /lib/libusb.so
ln -sr /usr/lib/libneon.so.*.*      /usr/lib/libneon.so

# Enable BIOS
mkdir -p /etc/malamute
cp /usr/share/bios/examples/config/malamute/malamute.cfg /etc/malamute
systemctl enable malamute
systemctl preset-all
systemctl disable bios-driver-netmon

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

# Enable rest API
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

# Setup zabbix
for i in mysql tntnet@bios malamute bios-db bios-server-agent bios-agent-inventory bios-agent-nut bios-driver-netmon nut-driver nut-monitor systemd-journald; do
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


# Set lang
echo 'export LANG="C"' > /etc/profile.d/lang.sh
for V in LANG LANGUAGE LC_ALL ; do echo "$V="'"C"'; done > /etc/default/locale

# More space saving
SPACERM="rm -rf"
$SPACERM /usr/share/nmap/nmap-os-db /usr/bin/{aria_read_log,aria_dump_log,aria_ftdump,replace,resolveip,myisamlog,myisam_ftdump}
[ "$SPACERM" \!= "rm" ] || install -m 0755 /usr/share/bios/scripts/resolveip.sh /usr/bin/resolveip
for i in /usr/share/mysql/* /usr/share/locale /usr/share/bios/{docker,examples,develop}; do
   [ -f "$i" ] || \
   [ "$i" = /usr/share/mysql/charsets ] || \
   [ "$i" = /usr/share/mysql/english ] || \
   $SPACERM "$i"
done

LANG=C date > /usr/share/bios-web/image-version.txt

# Show the package list
dpkg --get-selections
dpkg-query -Wf '${Installed-Size}\t${Package}\n' | sort -n

# Get rid of static qemu binaries needed for crossinstallation
# TODO: Integrate this better into
#       build-recipe-preinstallimage/init_buildsystem
rm -f /usr/bin/qemu*

[ -s "/usr/share/bios/.git_details" ] && grep ESCAPE "/usr/share/bios/.git_details" > /usr/share/bios-web/git_details.txt || echo "WARNING: Do not have /usr/share/bios/.git_details"

