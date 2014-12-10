FROM debian:jessie
MAINTAINER gguillaume "geraldguillaume@eaton.com"
RUN echo "Acquire::http::Proxy \"http://proxy.eur.etn.com:8080\";" > /etc/apt/apt.conf 
ENV http_proxy http://proxy.eur.etn.com:8080 
RUN apt-get update
#RUN apt-get install -y apt-utils
RUN apt-get install -y nut-server nut-client nut 
RUN apt-get install -y tntnet tntnet-runtime libtntnet12 libcxxtools9 libtntdb4 tntdb-mysql4 
RUN apt-get install -y libsasl2-2 sasl2-bin
RUN apt-get install -y libpgm-5.1-0 libsodium13
RUN apt-get install -y nmap
RUN apt-get install -y mysql-client 
RUN apt-get install -y sudo
RUN apt-get install -y vim-tiny
RUN apt-get clean

ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/cidrcalc_1.2.3-1_amd64.deb /tmp/cidrcalc_1.2.3-1_amd64.deb
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libcidr0_1.2.3-1_amd64.deb /tmp/libcidr0_1.2.3-1_amd64.deb
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libzmq3_4.0.4+dfsg-2_amd64.deb /tmp/libzmq3_4.0.4+dfsg-2_amd64.deb
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libczmq1_3.0.0~20141104git59a4f16-2_amd64.deb /tmp/libczmq1_3.0.0~20141104git59a4f16-1_amd64.deb
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libnutclient0_2.7.2-2_amd64.deb /tmp/libnutclient0_2.7.2-2_amd64.deb
RUN dpkg -i /tmp/libcidr0_1.2.3-1_amd64.deb
RUN dpkg -i /tmp/cidrcalc_1.2.3-1_amd64.deb
RUN dpkg -i /tmp/libzmq3_4.0.4+dfsg-2_amd64.deb
RUN dpkg -i /tmp/libczmq1_3.0.0~20141104git59a4f16-1_amd64.deb
RUN dpkg -i /tmp/libnutclient0_2.7.2-2_amd64.deb
RUN rm /tmp/*.deb

#Install BIOS binaries & config files
ADD tntnet.xml /usr/local/etc/tntnet.xml
ADD simple /usr/local/bin/simple
ADD netmon /usr/local/bin/netmon
ADD driver-nmap /usr/local/bin/driver-nmap
ADD bios_web.so.0.0.0 /usr/local/lib/bios_web.so.0.0.0
RUN ln -s /usr/local/lib/bios_web.so.0.0.0 /usr/local/lib/bios_web.so
RUN mkdir /usr/local/share/web
RUN mkdir /var/www

#User PAM
RUN mv /etc/default/saslauthd /etc/default/saslauthd.orig
ADD saslauthd /etc/default/saslauthd
RUN adduser --quiet --disabled-password -shell /bin/bash --home /home/admin --gecos "User" admin
RUN echo "admin\nadmin\n" | passwd admin 

#mysql configuration (assume link mysql:db with other mariadb container)
RUN mv /etc/mysql/my.cnf /etc/mysql/my.cnf.orig
RUN echo "[client]" > /etc/mysql/my.cnf
RUN echo "host            = db" >> /etc/mysql/my.cnf
RUN echo "port            = 3306" >> /etc/mysql/my.cnf
RUN echo "protocol        = TCP" >> /etc/mysql/my.cnf

#DB init script
ADD initdb.sql /tmp/initdb.sql
ADD load_data.sql /tmp/load_data.sql
ADD patch.sql /tmp/patch.sql

#Install NUT dumy drivers and config
ADD ups.conf /etc/nut/ups.conf
ADD upsmon.conf /etc/nut/upsmon.conf
ADD ups.dev /etc/nut/ups.dev
ADD UPS1-LAB.dev /etc/nut/UPS1-LAB.dev
ADD ePDU1-LAB.dev /etc/nut/ePDU1-LAB.dev
RUN echo "MODE=standalone" > /etc/nut/nut.conf
RUN chmod 644 /etc/nut/*
RUN chown root:nut /etc/nut/*

#prepare entry point
ADD start_bios.sh /usr/local/bin/start_bios.sh 
#ENTRYPOINT ["/bin/bash", "-e", "/usr/local/bin/start_bios.sh"] 

EXPOSE 8000
 
