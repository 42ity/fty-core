#!/bin/sh

BASEDIR="`pwd`"
mkdir -p data/{datacenter,room,row,rack}
pushd data/datacenter
"$BASEDIR"/fake_message << EOF
1
0
test_dc
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
1
alpha
description
Empty_room
exit
EOF
"$BASEDIR"/fake_message << EOF
3
1
beta
description
Real_room
exit
EOF
popd
pushd data/row
"$BASEDIR"/fake_message << EOF
4
3
front_one
description
Real_room
exit
EOF
popd
pushd data/rack
"$BASEDIR"/fake_message << EOF
5
4
in_the_row
brand
Eaton
u_size
24
exit
EOF
"$BASEDIR"/fake_message << EOF
6
3
sole_rack
brand
APC
u_size
6
exit
EOF
