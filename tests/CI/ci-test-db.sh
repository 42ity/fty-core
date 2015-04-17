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

loaddb_file() {
    mysql -u root < "$1" > /dev/null || \
        CODE=$? die "Could not load $1"
}

set -u
set -e

DB_LOADDIR="$CHECKOUTDIR/tools"

DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_TOPO="power_topology.sql"
DB_TOPO1="location_topology.sql"
DB_RACK_POWER="rack_power.sql"
DB_CRUD="crud_test.sql"

RESULT=0

echo "--------------- ensure bins to test --------------"
./autogen.sh --optseqmake --nodistclean ${AUTOGEN_ACTION_MAKE} \
    test-db test-db2 \
    test-db-asset-crud test-dbtopology test-totalpower \
    || true

echo "-------------------- reset db --------------------"
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_DATA"
echo "-------------------- test-db --------------------"
set +e
"$BUILDSUBDIR"/test-db
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db failed"
    echo "----------------------------------------"
    RESULT=1
fi
echo "-------------------- test-db2 --------------------"
"$BUILDSUBDIR"/test-db2
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db2 failed"
    echo "----------------------------------------"
    RESULT=1
fi

echo "-------------------- test-db-asset-crud-----"
echo "-------------------- reset db --------------------"
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_CRUD"
"$BUILDSUBDIR"/test-db-asset-crud
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db-asset-crud failed"
    echo "----------------------------------------"
    RESULT=1
fi

for P in "$DB_TOPO" "$DB_TOPO1"; do
    echo "-------------------- fill db for topology $P --------------------"
    loaddb_file "$DB_LOADDIR/$DB_BASE"
    loaddb_file "$DB_LOADDIR/$P"
    echo "-------------------- test-dbtopology $P --------------------"
    set +e
    "$BUILDSUBDIR"/test-dbtopology "[$P]"
    if [ "$?" != 0 ] ; then
        echo "----------------------------------------"
        echo "ERROR: test-dbtopology $P failed"
        echo "----------------------------------------"
        RESULT=1
    fi
done

echo "-------------------- test-total-power --------------------"
echo "-------------------- fill db for rack power --------------------"
loaddb_file "$DB_LOADDIR/$DB_BASE"
loaddb_file "$DB_LOADDIR/$DB_RACK_POWER"
"$BUILDSUBDIR"/test-totalpower "[$DB_RACK_POWER]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-totalpower failed"
    echo "----------------------------------------"
    RESULT=1
fi

cd -
exit $RESULT
