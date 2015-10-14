
#
# Copyright (C) 2015 Eaton
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


#! \file asset_racks.sh
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \brief Not yet documented file

echo "********* 1. No_datacenters_present ***************************************************************"
echo "***************************************************************************************************"
test_it "No_datacenters_present"
# delete all assets, no rooms are present
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools
loaddb_file "$DB_LOADDIR/$DB_BASE"

curlfail_push_expect_noerrors
api_get_json /asset/datacenters >&5
print_result $?
curlfail_pop

for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DATA" "$DB_DATA_TESTREST"; do
    loaddb_file "$DB_LOADDIR/$data" || return $?
done

echo "********* 2. Get_the_list_of_datacenters ****************************************************************"
echo "***************************************************************************************************"
test_it "Get_the_list_of_datacenters"
api_get_json /asset/datacenters >&5
print_result $?

echo "********* 3. no_rooms_present *********************************************************************"
echo "***************************************************************************************************"
# delete all assets, no rooms are present
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools
loaddb_file "$DB_LOADDIR/$DB_BASE"

test_it "no_rooms_present"
curlfail_push_expect_noerrors
api_get_json /asset/rooms >&5
print_result $?
curlfail_pop

#fill DB again
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools

for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DATA" "$DB_DATA_TESTREST"; do
    loaddb_file "$DB_LOADDIR/$data" || return $?
done

echo "********* 4. list_of_all_rooms ********************************************************************"
echo "***************************************************************************************************"
test_it "list_of_all_rooms"
api_get_json /asset/rooms >&5
print_result $?

echo "********* 5. no_rows_present *********************************************************************"
echo "***************************************************************************************************"
# delete all assets, no rows are present
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools
loaddb_file "$DB_LOADDIR/$DB_BASE"

test_it "no_rows_present"
curlfail_push_expect_noerrors
api_get_json /asset/rows >&5
print_result $?
curlfail_pop

#fill DB again
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools

for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DATA" "$DB_DATA_TESTREST"; do
    loaddb_file "$DB_LOADDIR/$data" || return $?
done

echo "********* 6. list_of_all_rows ********************************************************************"
echo "***************************************************************************************************"
test_it "list_of_all_rows"
api_get_json /asset/rows >&5
print_result $?


echo "********* 7. no_racks_present *********************************************************************"
echo "***************************************************************************************************"
# delete all assets, no racks are present
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools
loaddb_file "$DB_LOADDIR/$DB_BASE"

test_it "no_racks_present"
curlfail_push_expect_noerrors
api_get_json /asset/racks >&5
print_result $?
curlfail_pop

#fill DB again
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_LOADDIR=$BUILDSUBDIR/tools

for data in "$DB_BASE" "$DB_ASSET_TAG_NOT_UNIQUE" "$DB_DATA" "$DB_DATA_TESTREST"; do
    loaddb_file "$DB_LOADDIR/$data" || return $?
done

echo "********* 8. list_of_all_racks ********************************************************************"
echo "***************************************************************************************************"
test_it "list_of_all_racks"
api_get_json /asset/racks >&5
print_result $?

