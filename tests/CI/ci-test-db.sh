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

if [ "x$CHECKOUTDIR" = "x" ]; then
    SCRIPTDIR="$(cd "`dirname $0`" && pwd)" || \
    SCRIPTDIR="`dirname $0`"
    case "$SCRIPTDIR" in
        */tests/CI|tests/CI)
           CHECKOUTDIR="$( echo "$SCRIPTDIR" | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
fi
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

[ -z "$BUILDSUBDIR" -o ! -d "$BUILDSUBDIR" ] && BUILDSUBDIR="$CHECKOUTDIR"
[ ! -x "$BUILDSUBDIR/config.status" ] && BUILDSUBDIR="$PWD"
if [ ! -x "$BUILDSUBDIR/config.status" ]; then
    echo "Cannot find $BUILDSUBDIR/config.status, did you run configure?"
    echo "Search path: $CHECKOUTDIR, $PWD"
    exit 1
fi
echo "CI-INFO: Using BUILDSUBDIR='$BUILDSUBDIR' to run the database tests"

set -u
set -e

DB_LOADDIR="$CHECKOUTDIR/tools"

DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_TOPO="power_topology.sql"
DB_TOPO1="location_topology.sql"
DB_RACK_POWER="rack_power.sql"

RESULT=0

cd $CHECKOUTDIR
echo "-------------------- reset db --------------------"
mysql -u root < "$DB_LOADDIR/$DB_BASE"
mysql -u root < "$DB_LOADDIR/$DB_DATA"
echo "-------------------- test-db --------------------"
set +e
make -C "$BUILDSUBDIR" test-db && "$BUILDSUBDIR"/test-db
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db failed"
    echo "----------------------------------------"
    RESULT=1
fi
echo "-------------------- test-db2 --------------------"
make -C "$BUILDSUBDIR" test-db2 && "$BUILDSUBDIR"/test-db2
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db2 failed"
    echo "----------------------------------------"
    RESULT=1
fi

make -C "$BUILDSUBDIR" test-dbtopology
for P in "$DB_TOPO" "$DB_TOPO1"; do
    echo "-------------------- fill db for topology $P --------------------"
    mysql -u root < "$DB_LOADDIR/$DB_BASE"
    mysql -u root < "$DB_LOADDIR/$P"
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
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-dbtopology failed"
    echo "----------------------------------------"
    RESULT=1
fi

echo "-------------------- test-total-power --------------------"
echo "-------------------- fill db for rack power --------------------"
make -C "$BUILDSUBDIR" test-totalpower 
mysql -u root < "$DB_LOADDIR/$DB_BASE"
mysql -u root < "$DB_LOADDIR/$DB_RACK_POWER"
"$BUILDSUBDIR"/test-totalpower "[$DB_RACK_POWER]"
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-totalpower failed"
    echo "----------------------------------------"
    RESULT=1
fi

cd -
exit $RESULT
