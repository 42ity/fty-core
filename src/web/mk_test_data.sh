#!/bin/sh

BASEDIR="`pwd`"
mkdir -p data/{datacenter,room,row,rack}
pushd data/datacenter
"$BASEDIR"/fake_message << EOF
1
0
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
1
alpha
1
0
description
Empty_room
exit
EOF
"$BASEDIR"/fake_message << EOF
3
1
beta
1
0
description
Real_room
exit
EOF
popd
pushd data/row
"$BASEDIR"/fake_message << EOF
4
2
front_one
3
1
description
Real_room
exit
EOF
popd
pushd data/rack
"$BASEDIR"/fake_message << EOF
5
3
in_the_row
4
2
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
3
1
brand
APC
u_size
6
exit
EOF
