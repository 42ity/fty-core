#!/bin/sh

BASEDIR="`pwd`"
mkdir -p data/{datacenter,room,row,rack}
pushd data/datacenter
"$BASEDIR"/fake_message << EOF
1
datacenter
test_dc
0
0
country
France
company
Eaton
exit
EOF
popd
pushd data/room
"$BASEDIR"/fake_message << EOF
2
room
alpha
1
datacenter
description
Empty_room
exit
EOF
"$BASEDIR"/fake_message << EOF
3
room
beta
1
datacenter
description
Real_room
exit
EOF
popd
pushd data/row
"$BASEDIR"/fake_message << EOF
4
row
front_one
3
room
description
Real_room
exit
EOF
popd
pushd data/rack
"$BASEDIR"/fake_message << EOF
5
rack
in_the_row
4
row
brand
Eaton
u_size
24
exit
EOF
"$BASEDIR"/fake_message << EOF
7
rack
yar
4
row
exit
EOF
"$BASEDIR"/fake_message << EOF
6
rack
sole_rack
3
room
brand
APC
u_size
6
exit
EOF
