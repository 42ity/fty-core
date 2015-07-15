FROM debian:jessie
MAINTAINER gguillaume "geraldguillaume@eaton.com"
RUN echo "Acquire::http::Proxy \"http://proxy.eur.etn.com:8080\";" > /etc/apt/apt.conf 
RUN echo "deb http://obs.roz.lab.etn.com:82/Pool:/master/Debian_8.0 /" > /etc/apt/sources.list.d/95bios-obs-roz-deb8.list 
ENV http_proxy http://proxy.eur.etn.com:8080 
RUN apt-get update && apt-get install -y --force-yes \
  nut-server nut-client nut \
  tntnet tntnet-runtime libtntnet12 libcxxtools9 libtntdb4 tntdb-mysql4 \
  libsasl2-2 sasl2-bin \
  libpgm-5.1-0 libsodium13 \
  mysql-client \
  sudo \
  vim-tiny \
  libmagic1 \
  libcidr0 \
  libzmq4 \
  libczmq1 \
  libmlm1 \
  malamute \
  libnutclient0
  
#RUN apt-get install -y apt-utils
#RUN apt-get install -y nmap
RUN apt-get clean

#Install BIOS binaries & config files
RUN mv /etc/tntnet/tntnet.xml /etc/tntnet/tntnet.xml.orig
ADD ./usr/local/share/bios/examples/tntnet.xml.example /etc/tntnet/tntnet.xml
RUN sed -i 's/usr\/local\/share\/bios\/web/usr\/share\/bios-web/' /etc/tntnet/tntnet.xml
RUN mv /etc/malamute/malamute.cfg /etc/malamute/malamute.cfg.orig
ADD ./usr/local/share/bios/examples/config/malamute/malamute.cfg /etc/malamute/malamute.cfg
ADD ./usr/local/libexec/bios/agent-dbstore /usr/local/libexec/bios/agent-dbstore
ADD ./usr/local/libexec/bios/agent-netmon /usr/local/libexec/bios/agent-netmon
ADD ./usr/local/libexec/bios/driver-nmap /usr/local/libexec/bios/driver-nmap
ADD ./usr/local/libexec/bios/agent-nut /usr/local/libexec/bios/agent-nut
ADD ./usr/local/libexec/bios/agent-inventory /usr/local/libexec/bios/agent-inventory
ADD ./usr/local/libexec/bios/agent-alert /usr/local/libexec/bios/agent-alert
ADD ./usr/local/libexec/bios/agent-cm /usr/local/libexec/bios/agent-cm
ADD ./usr/local/libexec/bios/agent-tpower /usr/local/libexec/bios/agent-tpower
ADD ./usr/local/lib/bios/bios_web.so.0.0.0 /usr/local/lib/bios/bios_web.so.0.0.0
RUN ln -s /usr/local/lib/bios/bios_web.so.0.0.0 /usr/local/lib/bios/bios_web.so
ADD ./usr/local/lib/libbiosapi.so.0.0.0 /usr/local/lib/libbiosapi.so.0.0.0
RUN ln -s /usr/local/lib/libbiosapi.so.0.0.0 /usr/local/lib/libbiosapi.so.0
RUN ln -s /usr/local/lib/libbiosapi.so.0 /usr/local/lib/libbiosapi.so
RUN mkdir -p /usr/local/share/bios/web
ADD ./usr/share/compiled-webapp.tar /usr/share/
RUN mv /usr/share/compiled-webapp /usr/share/bios-web
#RUN mkdir /usr/share/bios-web
ENV LD_LIBRARY_PATH /usr/local/lib/

#User PAM
RUN sed -i 's/START=no/START=yes/' /etc/default/saslauthd
RUN adduser --quiet --disabled-password -shell /bin/bash --home /home/admin --gecos "User" admin
RUN echo "admin\nadmin\n" | passwd admin 

#mysql configuration (assume link mysql:db with other mariadb container)
#RUN mv /etc/mysql/my.cnf /etc/mysql/my.cnf.orig
RUN echo "[client]" > /etc/mysql/my.cnf
RUN echo "host            = db" >> /etc/mysql/my.cnf
RUN echo "port            = 3306" >> /etc/mysql/my.cnf
RUN echo "protocol        = TCP" >> /etc/mysql/my.cnf

#DB init script
ADD ./usr/local/share/bios/sql/mysql/initdb.sql /usr/local/share/bios/sql/mysql/initdb.sql
ADD ./usr/local/share/bios/examples/sql/mysql/load_data.sql /usr/local/share/bios/examples/sql/mysql/load_data.sql
ADD ./usr/local/share/bios/examples/sql/mysql/load_ROZLAB.sql /usr/local/share/bios/examples/sql/mysql/load_ROZLAB.sql
ADD ./usr/local/share/bios/docker/resources/patch.sql /usr/local/share/bios/docker/resources/patch.sql

#Install NUT dumy drivers and config
ADD ./usr/local/share/bios/docker/resources/ups.conf /etc/nut/ups.conf
ADD ./usr/local/share/bios/docker/resources/upsmon.conf /etc/nut/upsmon.conf
ADD ./usr/local/share/bios/docker/resources/UPS1-LAB.dev /etc/nut/UPS1-LAB.dev
ADD ./usr/local/share/bios/docker/resources/UPS2-LAB.dev /etc/nut/UPS2-LAB.dev
ADD ./usr/local/share/bios/docker/resources/ePDU1-LAB.dev /etc/nut/ePDU1-LAB.dev
ADD ./usr/local/share/bios/docker/resources/ePDU2-LAB.dev /etc/nut/ePDU2-LAB.dev
ADD ./usr/local/share/bios/docker/resources/ROZ.UPS1.dev /etc/nut/ROZ.UPS1.dev
ADD ./usr/local/share/bios/docker/resources/ROZ.ePDU04.dev /etc/nut/ROZ.ePDU04.dev
ADD ./usr/local/share/bios/docker/resources/ROZ.ePDU05.dev /etc/nut/ROZ.ePDU05.dev
RUN echo "MODE=standalone" > /etc/nut/nut.conf
RUN chmod 644 /etc/nut/*
RUN chown root:nut /etc/nut/*

#prepare entry point
ADD ./usr/local/share/bios/docker/resources/start_bios.sh /usr/local/libexec/bios/start_bios.sh 
#ENTRYPOINT ["/bin/bash", "-e", "/usr/local/libexec/bios/start_bios.sh"] 

EXPOSE 8000

