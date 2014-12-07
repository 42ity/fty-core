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
#
# Description: ??
#
# Requirements:
#   - mariadb running
#   - db user root without password

[ "x$CHECKOUTDIR" = "x" ] && \
    case "`dirname $0`" in
       */tests/CI|tests/CI)
           CHECKOUTDIR="$( cd `dirname $0`; pwd | sed 's|/tests/CI$||' )" || \
           CHECKOUTDIR="" ;;
    esac
[ "x$CHECKOUTDIR" = "x" ] && CHECKOUTDIR=~/project
echo "INFO: Test '$0 $@' will (try to) commence under CHECKOUTDIR='$CHECKOUTDIR'..."

set -u
set -e

DB_BASE="$CHECKOUTDIR/tools/initdb.sql"
DB_DATA="$CHECKOUTDIR/tools/load_data.sql"
DB_TOPO="$CHECKOUTDIR/tools/power_topology.sql"
RESULT=0

cd $CHECKOUTDIR
echo "-------------------- reset db --------------------"
mysql -u root < "$DB_BASE"
mysql -u root < "$DB_DATA"
echo "-------------------- test-db --------------------"
set +e
make test-db && ./test-db
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db failed"
    echo "----------------------------------------"
    RESULT=1
fi
echo "-------------------- test-db2 --------------------"
make test-db2 && ./test-db2
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-db2 failed"
    echo "----------------------------------------"
    RESULT=1
fi

echo "-------------------- fill db for topology --------------------"
set -e
mysql -u root < "$DB_BASE"
mysql -u root < "$DB_TOPO"
echo "-------------------- test-dbtopology --------------------"
set +e
# make test-dbtopology && ./test-dbtopology
if [ "$?" != 0 ] ; then
    echo "----------------------------------------"
    echo "ERROR: test-dbtopology failed"
    echo "----------------------------------------"
    RESULT=1
fi
cd -
exit $RESULT
