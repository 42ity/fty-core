BuildRequires: bison debhelper flex pkg-config autoconf automake libtool
BuildRequires: gdb strace gawk ccache git
BuildRequires: iptables-dev libatm1-dev libdb-dev lynx lynx-cur
BuildRequires: libselinux1-dev linux-libc-dev linuxdoc-tools
BuildRequires: asciidoc docbook-xsl-ns docbook-xsl docbook-xml libxml2-utils xsltproc
BuildRequires: libzmq4-dev libczmq-dev libjsoncpp-dev libcxxtools-dev libtntdb-dev
BuildRequires: libcidr0-dev libvariant-dev libtntnet-dev tntnet-runtime tntnet
BuildRequires: libsasl2-dev libsodium-dev libmlm-dev libmagic-dev
BuildRequires: libnutclient-dev libnutscan-dev python vim-tiny man
#

#!SimpleImage
Name: devel-image
BuildRequires: iproute iptables udhcpc openssh-server bash ifupdown udev tntnet ntp watchdog
BuildRequires: iputils-ping apt udev mtd-utils systemd core sudo nut nut-snmp nut-xml bios-web
BuildRequires: tntdb-mysql4 libtntdb4 systemd-sysv malamute mariadb-server-10.0 sasl2-bin
BuildRequires: mime-support ncurses-base libgdbm3 zabbix-agent msmtp u-boot-tools ifplugd
BuildRequires: augeas-tools augeas-lenses
BuildRequires: libpam-cracklib libcrack2 cracklib-runtime wamerican

%build
IMGTYPE=devel /usr/share/bios/obs/preinstallimage-bios.sh
