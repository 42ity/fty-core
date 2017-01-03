#!/usr/bin/env bash

################################################################################
#  This file is copied from fty-asset/ci_build.sh                              #
################################################################################

set -x
set -e

if [ "$BUILD_TYPE" == "default" ] || [ "$BUILD_TYPE" == "default-Werror" ] ; then
    if [ -d "./tmp" ]; then
        rm -rf ./tmp
    fi
    mkdir -p tmp
    BUILD_PREFIX=$PWD/tmp

    CONFIG_OPTS=()
    COMMON_CFLAGS=""
    EXTRA_CFLAGS=""
    EXTRA_CPPFLAGS=""
    EXTRA_CXXFLAGS=""
    if [ "$BUILD_TYPE" == "default-Werror" ] ; then
        COMPILER_FAMILY=""
        if [ -n "$CC" -a -n "$CXX" ]; then
            if "$CC" --version 2>&1 | grep GCC > /dev/null && \
               "$CXX" --version 2>&1 | grep GCC > /dev/null \
            ; then
                COMPILER_FAMILY="GCC"
            fi
        else
            if "gcc" --version 2>&1 | grep GCC > /dev/null && \
               "g++" --version 2>&1 | grep GCC > /dev/null \
            ; then
                # Autoconf would pick this by default
                COMPILER_FAMILY="GCC"
            elif "cc" --version 2>&1 | grep GCC > /dev/null && \
               "c++" --version 2>&1 | grep GCC > /dev/null \
            ; then
                COMPILER_FAMILY="GCC"
            fi
        fi

        case "${COMPILER_FAMILY}" in
            GCC)
                echo "NOTE: Enabling ${COMPILER_FAMILY} compiler pedantic error-checking flags for BUILD_TYPE='$BUILD_TYPE'" >&2
                COMMON_CFLAGS="-Wall -Werror"
                EXTRA_CFLAGS="-std=c99"
                EXTRA_CPPFLAGS=""
                EXTRA_CXXFLAGS="-std=c++99"
                ;;
            *)
                echo "WARNING: Current compiler is not GCC, not enabling pedantic error-checking flags for BUILD_TYPE='$BUILD_TYPE'" >&2
                ;;
        esac
    fi
    CONFIG_OPTS+=("CFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CFLAGS}")
    CONFIG_OPTS+=("CPPFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CPPFLAGS}")
    CONFIG_OPTS+=("CXXFLAGS=-I${BUILD_PREFIX}/include ${COMMON_CFLAGS} ${EXTRA_CXXFLAGS}")
    CONFIG_OPTS+=("LDFLAGS=-L${BUILD_PREFIX}/lib")
    CONFIG_OPTS+=("PKG_CONFIG_PATH=${BUILD_PREFIX}/lib/pkgconfig")
    CONFIG_OPTS+=("--prefix=${BUILD_PREFIX}")
    CONFIG_OPTS+=("--with-docs=no")
    CONFIG_OPTS+=("--quiet")

    # Clone and build dependencies
    git clone --quiet --depth 1 https://github.com/zeromq/libzmq.git libzmq.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 https://github.com/zeromq/czmq.git czmq.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 https://github.com/zeromq/malamute.git malamute.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 https://github.com/42ity/libmagic magic.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    BASE_PWD=${PWD}
    git clone --quiet --depth 1 https://github.com/42ity/libcidr cidr
    cd cidr
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 -b 42ity https://github.com/42ity/cxxtools cxxtools.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 -b 1.3 https://github.com/42ity/tntdb tntdb.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"
    git clone --quiet --depth 1 https://github.com/42ity/fty-proto fty-proto.git
    BASE_PWD=${PWD}
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
    cd "${BASE_PWD}"

    # Build and check this project
    ./autogen.sh 2> /dev/null
    ./configure --enable-drafts=yes "${CONFIG_OPTS[@]}"
    make VERBOSE=1 all

    echo "=== Are GitIgnores good after 'make all' with drafts? (should have no output below)"
    git status -s || true
    echo "==="

    if [ "$BUILD_TYPE" == "default-Werror" ] ; then
        echo "NOTE: Skipping distcheck for BUILD_TYPE='$BUILD_TYPE'" >&2
    else
        export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=yes ${CONFIG_OPTS[@]}"
        make VERBOSE=1 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS" distcheck

        echo "=== Are GitIgnores good after 'make distcheck' with drafts? (should have no output below)"
        git status -s || true
        echo "==="
    fi

    # Build and check this project without DRAFT APIs
    make distclean

    git clean -f
    git reset --hard HEAD
    (
        ./autogen.sh 2> /dev/null
        ./configure --enable-drafts=no "${CONFIG_OPTS[@]}" --with-docs=yes
        make VERBOSE=1 all || exit $?
        if [ "$BUILD_TYPE" == "default-Werror" ] ; then
            echo "NOTE: Skipping distcheck for BUILD_TYPE='$BUILD_TYPE'" >&2
        else
            export DISTCHECK_CONFIGURE_FLAGS="--enable-drafts=no ${CONFIG_OPTS[@]} --with-docs=yes" && \
            make VERBOSE=1 DISTCHECK_CONFIGURE_FLAGS="$DISTCHECK_CONFIGURE_FLAGS" distcheck || exit $?
        fi
    ) || exit 1

    echo "=== Are GitIgnores good after 'make distcheck' without drafts? (should have no output below)"
    git status -s || true
    echo "==="

elif [ "$BUILD_TYPE" == "bindings" ]; then
    pushd "./bindings/${BINDING}" && ./ci_build.sh
else
    pushd "./builds/${BUILD_TYPE}" && REPO_DIR="$(dirs -l +1)" ./ci_build.sh
fi
