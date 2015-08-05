#/!bin/sh

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Author(s): Barbora Stepankova <BarboraStepankova@Eaton.com>,
#            Tomas Halman <TomasHalman@eaton.com>
#            Alena Chernikava <AlenaChernikava@eaton.com>
#
# Description:
#       Every database test has its own requirements regarding input data.
#       That is why before every database test database should be initialised
#       and apropriate testing data should be filled.
#
# Requirements:
#   - mariadb running
#   - db user root without password

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=yes determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the database tests"

set -u
set -e

DB_LOADDIR="$CHECKOUTDIR/tools"

DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_TOPO="power_topology.sql"
DB_TOPO1="location_topology.sql"
DB_RACK_POWER="rack_power.sql"
DB_DC_POWER="dc_power.sql"
DB_CRUD="crud_test.sql"
DB_OUTAGE="test_outage.sql"
DB_ALERT="test_alert.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"

RESULT=0
FAILED=""

trap_exit() {
    # Capture the exit code first, if any was set by an exit() or "set -e".
    # Prefer to use the program-defined error code if any was set, though.
    TRAP_RESULT=$?
    [ "$RESULT" -gt 0 ] && TRAP_RESULT="$RESULT"

    if [ -n "$FAILED" ]; then
        logmsg_error "The following tests have failed:"
        for F in $FAILED; do echo " * $F" >&2; done
        [ "$TRAP_RESULT" = 0 ] || TRAP_RESULT=1
    fi

    cd -
    exit $TRAP_RESULT
}

trap "trap_exit" EXIT SIGHUP SIGINT SIGQUIT SIGTERM

echo "-------- ensure bins to test are up to date -------"
./autogen.sh --optseqmake --nodistclean ${AUTOGEN_ACTION_MAKE} \
    test-db2 test-db-alert \
    test-db-asset-crud test-dbtopology test-outage test-totalpower \
    || FAILED="compilation"
sleep 1

# From here on we use the build directory since libtool-generated
# scripts which wrap our build products may want that
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"

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

loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_DATA"
echo "-------------------- test-db2 --------------------"
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
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
loaddb_file "$DB_LOADDIR/$DB_ALERT"
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
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
loaddb_file "$DB_LOADDIR/$DB_CRUD"
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

for P in "$DB_TOPO" "$DB_TOPO1"; do
    echo "-------------------- fill db for topology $P --------------------"
    loaddb_file "$DB_LOADDIR/$DB_BASE"
    loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
    loaddb_file "$DB_LOADDIR/$P"
    echo "-------------------- test-dbtopology $P --------------------"
    set +e
    "$BUILDSUBDIR"/test-dbtopology "[$P]"
    if [ "$?" != 0 ] ; then
        echo "----------------------------------------"
        echo "ERROR: test-dbtopology $P failed"
        echo "----------------------------------------"
        RESULT=1
        FAILED="$FAILED test-dbtopology::$P"
        [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
    fi
    sleep 1
done

echo "-------------------- test-total-power --------------------"
echo "-------------------- fill db for rack power --------------------"
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
loaddb_file "$DB_LOADDIR/$DB_RACK_POWER"
"$BUILDSUBDIR"/test-totalpower "[$DB_RACK_POWER]"
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
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
loaddb_file "$DB_LOADDIR/$DB_DC_POWER"
"$BUILDSUBDIR"/test-totalpower "[$DB_DC_POWER]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-totalpower failed"
    echo "----------------------------------------"
    RESULT=1
    FAILED="$FAILED test-totalpower::dc"
    [ x"$CITEST_QUICKFAIL" = xyes ] && exit $RESULT
fi
sleep 1

loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_ASSET_TAG_NOT_UNIQUE"
loaddb_file "$DB_LOADDIR/$DB_OUTAGE"
echo "-------------------- test-db-outage --------------------"
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
