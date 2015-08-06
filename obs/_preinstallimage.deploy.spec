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
BuildRequires: mime-support ncurses-base libgdbm3 zabbix-agent msmtp u-boot-tools ifplugd
BuildRequires: augeas-tools augeas-lenses
BuildRequires: libpam-cracklib libcrack2 cracklib-runtime wamerican

%build
IMGTYPE=deploy /usr/share/bios/obs/preinstallimage-bios.sh
