BuildRequires: bison  debhelper   flex  iptables-dev  libatm1-dev  libdb-dev  libselinux1-dev  linux-libc-dev  linuxdoc-tools  lynx   lynx-cur  pkg-config  libzmq4-dev  autoconf  automake  libtool  libjsoncpp-dev  asciidoc  docbook-xsl-ns  docbook-xsl  docbook-xml  libxml2-utils  xsltproc  libcxxtools-dev  libtntdb-dev  libcidr0-dev  libvariant-dev  libczmq-dev  libnutclient-dev  libtntnet-dev  tntnet-runtime  tntnet  libsasl2-dev  libsodium-dev  libmlm-dev  libmagic-dev  libnutscan-dev python vim-tiny man gdb strace gawk ccache git
#

#!SimpleImage
Name: devel-image
BuildRequires: iproute iptables udhcpc openssh-server bash ifupdown udev tntnet ntp watchdog
BuildRequires: iputils-ping apt udev mtd-utils systemd core sudo nut nut-snmp nut-xml bios-web
BuildRequires: tntdb-mysql4 libtntdb4 systemd-sysv malamute mariadb-server-10.0 sasl2-bin
BuildRequires: mime-support ncurses-base libgdbm3 zabbix-agent msmtp u-boot-tools ifplugd augeas-tools augeas-lenses
BuildRequires: libpam-cracklib libcrack2 cracklib-runtime wamerican

%build
IMGTYPE=devel /usr/share/bios/obs/preinstallimage-bios.sh
