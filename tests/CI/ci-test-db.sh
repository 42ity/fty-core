#/!bin/sh

# Copyright (C) 2014-2015 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    ci-test-db.sh
#  \brief   tests database setup
#  \author  Barbora Stepankova <BarboraStepankova@Eaton.com>
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \author  Alena Chernikava <AlenaChernikava@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Every database test has its own requirements regarding input data.
#       That is why before every database test database should be initialised
#       and appropriate testing data should be filled.
#
# Requirements:
#   - mariadb running
#   - db user root without password
#
# TODO: rewrite to use test_it()

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no NEED_CHECKOUTDIR=yes determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build the database tests"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

set -u
set -e

RESULT=0
FAILED=""

trap_exit() {
    # Capture the exit code first, if any was set by an exit() or "set -e".
    # Prefer to use the program-defined error code if any was set, though.
    TRAP_RESULT=$?
    [ "$RESULT" -gt 0 ] && TRAP_RESULT="$RESULT"

    if [ -n "$FAILED" ]; then
        logmsg_error "The following tests have failed:"
        for F in $FAILED; do echo " * FAILED  $F" >&2; done
        [ "$TRAP_RESULT" = 0 ] || TRAP_RESULT=1
    fi

    cd -
    exit $TRAP_RESULT
}

settraps "trap_exit"

echo "-------- ensure bins to test are up to date -------"
[ -n "$BUILDSUBDIR" ] && [ -x "$BUILDSUBDIR/config.status" ] || \
    ./autogen.sh --install-dir / --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
    ${AUTOGEN_ACTION_CONFIG}

./autogen.sh --optseqmake --nodistclean ${AUTOGEN_ACTION_MAKE} \
    test-db2 test-db-alert \
    test-db-asset-crud test-dbtopology test-outage test-totalpower \
    || FAILED="compilation"
sleep 1

# From here on we use the build directory since libtool-generated
# scripts which wrap our build products may want that
NEED_BUILDSUBDIR=yes determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the database tests"

echo "------------- empty the db before tests ----------"
${CHECKOUTDIR}/tests/CI/ci-empty-db.sh || \
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "FATAL: ci-empty-db.sh preparation failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED ci-empty-db.sh"
    die "Can't prepare the database"
fi
sleep 1

echo "-------------------- test-db2 --------------------"
echo "-------------------- reset db --------------------"
loaddb_file "$DB_BASE" \
&& loaddb_file "$DB_DATA" \
|| die "Can't prepare the database"
"$BUILDSUBDIR"/test-db2
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db2 failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-db2"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-db-alert --------------------"
echo "-------------------- reset db --------------------"
loaddb_file "$DB_BASE" \
&& loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" \
&& loaddb_file "$DB_ALERT" \
|| die "Can't prepare the database"
"$BUILDSUBDIR"/test-db-alert
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db-alert failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-db-alert"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-db-asset-crud-----"
echo "-------------------- reset db --------------------"
loaddb_file "$DB_BASE" \
&& loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" \
&& loaddb_file "$DB_CRUD" \
|| die "Can't prepare the database"
"$BUILDSUBDIR"/test-db-asset-crud
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db-asset-crud failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-db-asset-crud"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-dbtopology location --------------------"
echo "-------------------- fill db for topology loaction --------------------"
loaddb_initial || die "Can't prepare the database"
loaddb_topo_loc || die "Can't prepare the database"
set +e
"$BUILDSUBDIR"/test-dbtopology "[$DB_TOPOL_NAME]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-dbtopology location failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-dbtopology::location"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-dbtopology power --------------------"
echo "-------------------- fill db for topology power --------------------"
loaddb_initial || die "Can't prepare the database"
loaddb_topo_pow || die "Can't prepare the database"
set +e
"$BUILDSUBDIR"/test-dbtopology "[$DB_TOPOP_NAME]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-dbtopology power failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-dbtopology::power"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-total-power --------------------"
echo "-------------------- fill db for rack power --------------------"
loaddb_rack_power || die "Can't prepare the database"
"$BUILDSUBDIR"/test-totalpower "[$DB_RACK_POWER_NAME]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-totalpower failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-totalpower::rack"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-total-power --------------------"
echo "-------------------- fill db for dc power --------------------"
loaddb_dc_power || die "Can't prepare the database"
"$BUILDSUBDIR"/test-totalpower "[$DB_DC_POWER_NAME]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-totalpower failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-totalpower::dc"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

echo "-------------------- test-db-outage --------------------"
echo "-------------------- fill db for outage ----------------------"
loaddb_file "$DB_BASE" \
&& loaddb_file "$DB_ASSET_TAG_NOT_UNIQUE" \
&& loaddb_file "$DB_OUTAGE" \
|| die "Can't prepare the database"
"$BUILDSUBDIR"/test-outage
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-outage failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-outage"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

# The trap-handler should display the summary (if any)
exit $RESULT
