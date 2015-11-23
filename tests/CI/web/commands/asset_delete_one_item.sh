#
# Copyright (c) 2015 Eaton
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
#! \file   asset_delete_one_item.sh
#  \brief  CI tests for asset delete calls
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \author Alena Chernikava <AlenaChernikava@Eaton.com>
echo
echo "###################################################################################################"
echo "********* asset_delete_one_item.sh ***************** START ****************************************"
echo "###################################################################################################"
echo

find_id() {
	local __CALL=$(api_get_json /asset/$1)
	local __REPLY=$(echo "$__CALL" | $JSONSH -x id)
	ID_1=$(echo "${__REPLY}" | cut -d '"' -f 6)
	ID=$(echo $ID_1 | cut -d ' ' -f $2)
	echo $ID
}

# Assumption: initdb + load_data files are uploaded.
# So, make sure this is  true;
DB_LOADDIR="$CHECKOUTDIR/database/mysql"
DB_INIT_delete="$DB_LOADDIR/initdb.sql"
DB_LOAD_DATA_delete="$DB_LOADDIR/load_data.sql"
loaddb_file "$DB_INIT_delete" || exit $?
loaddb_file "$DB_LOAD_DATA_delete" || exit $?

# Need to check, number of expected rows in the table
ASSETS_NUMBER="$(mysql -u root box_utf8 <<< 'select count(id) as assets_count from v_bios_asset_element')"
echo $ASSETS_NUMBER
echo "expected 35"

# Start
No=1

echo "********* asset_delete_one_item.sh ****************************************************************"
echo "********* ${No}. Delete_devices_with_non_existing_ID **************************************************"
echo "***************************************************************************************************"
test_it "Delete_devices_with non-existing ID"
curlfail_push_expect_404
api_auth_delete_json /asset/10000000 >&5
print_result $?
curlfail_pop
No="$(expr $No + 1)"

echo "********* asset_delete_one_item.sh ****************************************************************"
echo "********* ${No}. Delete_devices_which_ID_is_0 *********************************************************"
echo "***************************************************************************************************"
test_it "Delete_devices_which_ID_is_0"
curlfail_push_expect_404
api_auth_delete_json /asset/0 >&5
print_result $?
curlfail_pop
No="$(expr $No + 1)"

echo "********* asset_delete_one_item.sh ****************************************************************"
echo "********* ${No}. Delete_devices_without_id ************************************************************"
echo "***************************************************************************************************"
test_it "Delete_devices_without_id"
curlfail_push_expect_400
api_auth_delete_json /asset/ >&5
print_result $?
curlfail_pop
No="$(expr $No + 1)"

# XXX this doesn't work correctly
# json in res is incorrect for now
# expected SUCCESS for now, but it should be 404
echo "********* asset_delete_one_item.sh ****************************************************************"
echo "********* ${No}. Unauthorized_delete_devices **********************************************************"
echo "***************************************************************************************************"
test_it "Unauthorized_delete_devices"
#curlfail_push_expect_404
api_delete_json /asset/31  >&5
print_result $?
#curlfail_pop
No="$(expr $No + 1)"

echo "********* asset_delete_one_item.sh ****************************************************************"
echo "********* ${No}. Delete_devices_which_ID_is_negative **************************************************"
echo "***************************************************************************************************"
test_it "Delete_devices_which_ID_is_negative"
curlfail_push_expect_404
api_auth_delete_json /asset/-1 >&5
print_result $?
curlfail_pop
No="$(expr $No + 1)"

# Start od deleting from up to down
for ent in datacenters rooms rows racks groups devices; do
   i=1
   ITEM_ID="`find_id $ent $i`"
   until [ -z "$ITEM_ID" ]
   do
      echo "********* asset_delete_one_item.sh ****************************************************************"
      echo "********* ${No}. Delete_${ent}_with_ID_=_${ITEM_ID} ******************************************************"
      echo "***************************************************************************************************"
      test_it "Delete_${ent}_with_ID_=_${ITEM_ID}"
      PARENTS_ID="$(mysql -u root box_utf8 <<< "select id from v_bios_asset_element where id_parent=$ITEM_ID")"
      if [[ "$PARENTS_ID" == "" ]];then
         curlfail_push_expect_noerrors
         echo "${ent} with ID = $ITEM_ID is expected to be deleted."
      else
         curlfail_push_expect_409
         echo "ERROR 409. ${ent} with ID = $ITEM_ID is expected to be NOT deleted."
      fi
      api_auth_delete_json /asset/$ITEM_ID >&5
      print_result $?
      curlfail_pop
      No="$(expr $No + 1)"
      i="$(expr $i + 1)"
      ITEM_ID="`find_id $ent $i`"
   done
done

# Start of deleting from down to up
for ent in devices groups racks rows rooms datacenters; do
   i=1
   ITEM_ID=`find_id $ent $i`
   until [ -z "$ITEM_ID" ]
   do
      echo "********* asset_delete_one_item.sh ****************************************************************"
      echo "********* ${No}. Delete_${ent}_with_ID_=_${ITEM_ID} ***********************************************"
      echo "***************************************************************************************************"
      test_it "Delete_${ent}_with_ID_=_${ITEM_ID}"
      PARENTS_ID=$(mysql -u root box_utf8 <<< "select id from v_bios_asset_element where id_parent=${ITEM_ID}")
      if [[ "$PARENTS_ID" == "" ]];then
         curlfail_push_expect_noerrors
         echo "${ent} with ID = ${ITEM_ID} is expected to be deleted."
      else
         curlfail_push_expect_409
         echo "ERROR 409. ${ent} with ID = ${ITEM_ID} is expected to be NOT deleted."
      fi
      api_auth_delete_json /asset/$ITEM_ID >&5
      print_result $?
      curlfail_pop
      No="$(expr $No + 1)"
      ITEM_ID=`find_id $ent $i`
   done
done

DEL_RES=0
for i in t_bios_asset_ext_attributes t_bios_asset_group_relation t_bios_asset_link t_bios_asset_ext_attributes t_bios_monitor_asset_relation ; do
    echo "********* asset_delete_one_item.sh ****************************************************************"
    echo "********* ${No}. Related_table ${i} must be empty ****************************"
    echo "***************************************************************************************************"
    test_it "Table ${i} must be empty"
    EXT_ATTR="$(mysql -u root box_utf8 <<< "select * from ${i}")"
    if [[ "$EXT_ATTR" != "" ]];then
        DEL_RES="$(expr $DEL_RES + 1)"
    fi
    print_result $DEL_RES
    No="$(expr $No + 1)"
done

echo
echo "###################################################################################################"
echo "********* asset_delete_one_item.sh ***************** END ******************************************"
echo "###################################################################################################"
echo
