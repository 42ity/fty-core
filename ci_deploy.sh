#!/usr/bin/env bash

################################################################################
#  This file is copied from fty-asset/ci_deploy.sh                             #
################################################################################

set -x
set -e

if [ "$BUILD_TYPE" == "default" ]; then
    # Tell travis to deploy all files in dist
    mkdir dist
    export FTY_CORE_DEPLOYMENT=dist/*
    # Move archives to dist
    mv *.tar.gz dist
    mv *.zip dist
    # Generate hash sums
    cd dist
    md5sum *.zip *.tar.gz > MD5SUMS
    sha1sum *.zip *.tar.gz > SHA1SUMS
    sha256sum *.zip *.tar.gz > SHA256SUMS
    cksum *.zip *.tar.gz > CKSUMS
    cd -
elif [ "$BUILD_TYPE" == "bindings" ] && [ "$BINDING" == "jni" ]; then
    ( cd bindings/jni && TERM=dumb PKG_CONFIG_PATH=/tmp/lib/pkgconfig ./gradlew clean bintrayUpload )
    cp bindings/jni/android/fty_core-android.jar fty_core-android-1.0.0.jar
    export FTY_CORE_DEPLOYMENT=fty_core-android-1.0.0.jar
else
    export FTY_CORE_DEPLOYMENT=""
fi
