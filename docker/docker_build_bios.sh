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
#   Description: Script to generate eaton/bios container
export build_dir=..
export temp_dir=./temp 

echo "preparing $temp_dir .. "
mkdir -p $temp_dir
CURDIR=`pwd`
cd $build_dir
make install
cd $CURDIR
cp ./resources/Dockerfile $temp_dir
cp /usr/share/compiled-webapp.tar $temp_dir/usr/share

#in case of network timeout between mbt and roz, force the way for getting libcidr0 and cidrcalc packages like this
#cp ../../temp/cidrcalc_1.2.3-1_amd64.deb $temp_dir
#cp ../../temp/libcidr0_1.2.3-1_amd64.deb $temp_dir

echo "building docker eaton/bios .."
sudo docker build --no-cache=true --rm=true -t eaton/bios $temp_dir

echo "cleaning .."
sudo docker rmi $(sudo docker images -q -f dangling=true)
sudo python ../tools/docker_clean_vfs.py
#rm -r $temp_dir

echo "done"
sudo docker images | grep eaton/bios

echo "exporting eaton/bios into eaton-bios.tar .."
sudo docker save eaton/bios > eaton-bios.tar
ls -lh  eaton-bios.tar
