#!/bin/bash
#
# Copyright (c) 2014-2015 Eaton
#
# This file is part of the Eaton $BIOS project.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    docker_run_bios.sh
#  \brief   Script to run eaton/bios container
#  \author  Gerald Guillaume <GeraldGuillaume@Eaton.com>

if sudo docker ps | grep dockerfile/mariadb > /dev/null
then
  echo "dockerfile/mariadb already started"
else
  echo "starting dockerfile/mariadb .."
  sudo docker run -d --name mysql dockerfile/mariadb
  sleep 10
fi
echo "starting eaton/bios .."
container_id=`sudo docker ps -a | grep bios | awk '{print $1}'`
if [[ "x$container_id" != "x" ]]
then
  sudo docker stop $container_id
  sudo docker rm $container_id 
fi
if [[ "x$1" == "x-ti" ]]
then
  sudo docker run -ti --cap-add=ALL --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash
else
  sudo docker run --cap-add=ALL --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash -c /usr/local/libexec/bios/start_bios.sh
fi
