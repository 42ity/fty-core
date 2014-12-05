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
# Description: tests database files import

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
set -x

DB1="$CHECKOUTDIR/tools/initdb.sql"
DB2="$CHECKOUTDIR/tools/load_data.sql"
DB2="$CHECKOUTDIR/tools/power_topology.sql"


cd $CHECKOUTDIR
echo "-------------------- reset db --------------------"
mysql -u root < "$DB1"
mysql -u root < "$DB2"
echo "-------------------- test-db --------------------"
make test-db && ./test-db
echo "-------------------- test-db2 --------------------"
make test-db2 && ./test-db2
echo "-------------------- fill db for topology --------------------"
mysql -u root < "$DB1"
mysql -u root < "$DB3"
echo "-------------------- test-dbtopology --------------------"
make test-dbtopology && ./test-dbtopology
