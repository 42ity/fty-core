#
# Copyright (c) 2015 Eaton
#
#T his program is free software; you can redistribute it and/or modify
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
# with this program; in case not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   asset_import_err_2.sh
#  \brief  CI tests for asset import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

_SCRIPT_NAME="asset_import"
# Add the library
. $CHECKOUTDIR/tests/CI/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"
RES_DIR="$CHECKOUTDIR/tests/CI/web/results"
DATASWITCH=0

initiate_db(){
   # Initiate DB
   DB_BASE="initdb.sql"
   loaddb_file "$CHECKOUTDIR/tools/$DB_BASE" || return $?
}

csv_import_err(){
   CSV_FILE_NAME=$1
   TABLE_NAME=$2
   NUM_EXPECTED=$3 # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
   FILENAME_PREFIX=$4
   ASSET="$CHECKOUTDIR/tools/asset_import/${CSV_FILE_NAME}"
   if [ "$NUM_EXPECTED" != 0 ];then
      if [ "$REZ" = 0 ];then
        if [ "$DATASWITCH" == 0 ];then
           #api_auth_post_file_json /asset/import assets=@$ASSET -H "Expect:"&& echo "$OUT_CURL" >&5
           api_auth_post_file /asset/import assets=@$ASSET -H "Expect:" && echo "$OUT_CURL" | $JSONSH -N  >&5
           REZ=$?
        else
           api_auth_post_file_data /asset/import assets=@$ASSET -H "Expect:" && echo "$OUT_CURL" | $JSONSH -N  >&5
           REZ=$?
        fi
        echo "REZ = $REZ"
      fi
   fi
}

test_tables(){
   CSV_FILE_NAME=$1
   NUM_EXPECTED=$2 # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
   FILENAME_PREFIX=$3
   csv_import_err "$CSV_FILE_NAME" "t_bios_asset_element" "$NUM_EXPECTED" "$FILENAME_PREFIX"
   echo "REZ1=$REZ"
   csv_import_err "$CSV_FILE_NAME" "t_bios_asset_group_relation" 0 "$FILENAME_PREFIX"
   echo "REZ2=$REZ"
   csv_import_err "$CSV_FILE_NAME" "t_bios_asset_ext_attributes" 0 "$FILENAME_PREFIX"
   echo "REZ3=$REZ"
   csv_import_err "$CSV_FILE_NAME" "t_bios_asset_link" 0 "$FILENAME_PREFIX"
   echo "REZ4=$REZ"
   csv_import_err "$CSV_FILE_NAME" "t_bios_asset_link_type" 0 "$FILENAME_PREFIX"
   echo "REZ5=$REZ"
}

echo
echo "###################################################################################################"
echo "********* asset_import_err_2.sh ******************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 1. Wrong maximum number of racks ********************************************************"
echo "***************************************************************************************************"
test_it "Wrong_maximum_number_of_racks"
initiate_db
REZ=0
test_tables "universal_asset_semicolon_max_num_rack_wrong_8.csv" 48 "ERROR"
print_result $REZ

