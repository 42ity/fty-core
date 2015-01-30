FROM debian:jessie
MAINTAINER gguillaume "geraldguillaume@eaton.com"
RUN echo "Acquire::http::Proxy \"http://proxy.eur.etn.com:8080\";" > /etc/apt/apt.conf 
RUN echo "deb http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0 /" > /etc/apt/sources.list.d/95bios-obs-roz-deb8.list 
ENV http_proxy http://proxy.eur.etn.com:8080 
RUN apt-get update
#RUN apt-get install -y apt-utils
RUN apt-get install -y --force-yes nut-server nut-client nut 
RUN apt-get install -y --force-yes tntnet tntnet-runtime libtntnet12 libcxxtools9 libtntdb4 tntdb-mysql4 
RUN apt-get install -y libsasl2-2 sasl2-bin
RUN apt-get install -y libpgm-5.1-0 libsodium13
RUN apt-get install -y nmap
RUN apt-get install -y mysql-client 
RUN apt-get install -y sudo
RUN apt-get install -y vim-tiny
RUN apt-get clean

RUN apt-get install -y --force-yes libcidr0
RUN apt-get install -y --force-yes cidrcalc

#RUN apt-get install -y --force-yes libzmq4
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libzmq4_4.2.0~20150120be23e699c9-1_amd64.deb /tmp/libzmq4_4.2.0~20150120be23e699c9-1_amd64.deb
RUN dpkg -i /tmp/libzmq4_4.2.0~20150120be23e699c9-1_amd64.deb

#RUN apt-get install -y --force-yes libczmq1
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libczmq1_3.0.0~20150120git5d02abb40-1_amd64.deb  /tmp/libczmq1_3.0.0~20150120git5d02abb40-1_amd64.deb 
RUN dpkg -i /tmp/libczmq1_3.0.0~20150120git5d02abb40-1_amd64.deb 

#RUN apt-get install -y --force-yes libmlm1
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libmlm1_1.1.0~20150120git28b4fdaca6-1_amd64.deb   /tmp/libmlm1_1.1.0~20150120git28b4fdaca6-1_amd64.deb  
RUN dpkg -i /tmp/libmlm1_1.1.0~20150120git28b4fdaca6-1_amd64.deb 

#RUN apt-get install -y --force-yes libnutclient0
ADD http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0/amd64/libnutclient0_2.7.2-2_amd64.deb   /tmp/libnutclient0_2.7.2-2_amd64.deb  
RUN dpkg -i /tmp/libnutclient0_2.7.2-2_amd64.deb

#Install BIOS binaries & config files
RUN mv /etc/tntnet/tntnet.xml /etc/tntnet/tntnet.xml.orig
ADD ./usr/local/share/core-0.1/examples/tntnet.xml.example /etc/tntnet/tntnet.xml
ADD ./usr/local/bin/simple /usr/local/bin/simple
ADD ./usr/local/bin/netmon /usr/local/bin/netmon
ADD ./usr/local/bin/driver-nmap /usr/local/bin/driver-nmap
RUN mkdir -p /usr/local/lib/core
ADD ./usr/local/lib/core/bios_web.so.0.0.0 /usr/local/lib/core/bios_web.so.0.0.0
RUN ln -s /usr/local/lib/core/bios_web.so.0.0.0 /usr/local/lib/core/bios_web.so
RUN mkdir -p /usr/local/share/core-0.1/web

#User PAM
RUN sed -i 's/START=no/START=yes/' /etc/default/saslauthd
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
ADD UPS1-LAB.dev /etc/nut/UPS1-LAB.dev
ADD UPS2-LAB.dev /etc/nut/UPS2-LAB.dev
ADD ePDU1-LAB.dev /etc/nut/ePDU1-LAB.dev
ADD ePDU2-LAB.dev /etc/nut/ePDU2-LAB.dev
RUN echo "MODE=standalone" > /etc/nut/nut.conf
RUN chmod 644 /etc/nut/*
RUN chown root:nut /etc/nut/*

#prepare entry point
ADD start_bios.sh /usr/local/bin/start_bios.sh 
#ENTRYPOINT ["/bin/bash", "-e", "/usr/local/bin/start_bios.sh"] 

EXPOSE 8000
 
