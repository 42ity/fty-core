#!/bin/sh
#
#   Copyright (c) 2015 Eaton Corporation <www.eaton.com>
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
#   Description: Script to generate eaton/bios container
set -e

export build_dir=..
export temp_dir=./temp 

echo "preparing $temp_dir .. "
mkdir -p "$temp_dir"
export DESTDIR=`cd $temp_dir; pwd`
CURDIR=`pwd`
cd $build_dir
make install

cd $CURDIR
cp ./resources/Dockerfile $temp_dir
sed -i "s|HTTP_PROXY|$http_proxy|g" $temp_dir/Dockerfile

mkdir -p "$temp_dir/usr/share"
cp ~/compiled-webapp.tar $temp_dir/usr/share

echo "building docker eaton/bios .."
sudo docker build --no-cache=true --rm=true -t eaton/bios $temp_dir

echo "cleaning .."
set +e
sudo docker rmi $(sudo docker images -q -f dangling=true)
sudo python $build_dir/tools/docker_clean_vfs.py
set -e
#rm -r $temp_dir

echo "done"
sudo docker images | grep eaton/bios

echo "exporting eaton/bios into eaton-bios.tar .."
sudo docker save eaton/bios > eaton-bios.tar
ls -lh  eaton-bios.tar
