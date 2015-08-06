#!/bin/sh
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
#   Description: Script to stop bios|mysql or all (both) containers from local cache
case "x$1" in
 "xall" | "x")
    $0 bios
    $0 mysql;; 
 *)
    echo "stopping $1 container .."
    container_id=`sudo docker ps -a | grep $1 | awk '{print $1}'`
    if [ "x$container_id" != "x" ]
    then
      sudo docker stop $container_id
      sudo docker ps -a | grep $1
    fi;;
esac

