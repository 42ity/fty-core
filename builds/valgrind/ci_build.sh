#!/usr/bin/env bash

################################################################################
#  This file is copied from fty-asset:buils/valgrind/ci_build.sh               #
################################################################################

set -x

cd ../..

mkdir tmp
BUILD_PREFIX=$PWD/tmp

CONFIG_OPTS=()
CONFIG_OPTS+=("CFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_OPTS+=("CPPFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_OPTS+=("CXXFLAGS=-I${BUILD_PREFIX}/include")
CONFIG_OPTS+=("LDFLAGS=-L${BUILD_PREFIX}/lib")
CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig")
CONFIG_OPTS+=("--prefix=${BUILD_PREFIX}")
CONFIG_OPTS+=("--with-docs=no")
CONFIG_OPTS+=("--quiet")

# Clone and build dependencies
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/libzmq.git libzmq.git
cd libzmq.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/czmq.git czmq.git
cd czmq.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/malamute.git malamute.git
cd malamute.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/libmagic magic.git
cd magic.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/cxxtools cxxtools.git
cd cxxtools.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/tntdb tntdb.git
cd tntdb.git/tntdb
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ../..
git clone --quiet --depth 1 -b release/IPM_Infra-1.3 https://github.com/42ity/fty-proto fty-proto.git
cd fty-proto.git
git --no-pager log --oneline -n1
if [ -e autogen.sh ]; then
    ./autogen.sh 2> /dev/null
fi
if [ -e buildconf ]; then
    ./buildconf 2> /dev/null
fi
./configure "${CONFIG_OPTS[@]}"
make -j4
make install
cd ..

# Build and check this project
./autogen.sh 2> /dev/null
./configure --enable-drafts=yes "${CONFIG_OPTS[@]}"
make VERBOSE=1 memcheck || exit 1
