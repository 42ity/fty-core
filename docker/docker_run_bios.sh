#!/bin/bash
#
#   Copyright (c) 2014 Eaton Corporation <www.eaton.com>
#   Copyright other contributors as noted in the AUTHORS file.
#
#   This file is part of the Eaton $BIOS project.
#
#   This is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This software is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#   Description: Script to run eaton/bios container
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
  sudo docker run -ti --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash
else
  sudo docker run --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash -c /usr/local/bin/start_bios.sh
fi
