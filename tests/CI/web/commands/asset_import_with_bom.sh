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
#! \file   asset_import_err.sh
#  \brief  CI tests for asset import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

_SCRIPT_NAME="asset_import"
# Add the library
. $CHECKOUTDIR/tests/CI/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"
RES_DIR="$CHECKOUTDIR/tests/CI/web/results"

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
         #api_auth_post_file_json /asset/import assets=@$ASSET -H "Expect:"&& echo "$OUT_CURL" >&5
         api_auth_post_file /asset/import assets=@$ASSET -H "Expect:"&& echo "$OUT_CURL" | $JSONSH -N  >&5
         REZ=$?
         #sleep 120
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
echo "********* asset_import_err.sh ********************** START ****************************************"
echo "###################################################################################################"
echo

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 1. Import universal tab 16BE_with_BOM ***************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_tab_16BE_with_BOM"
#curlfail_push_expect_400
initiate_db
REZ=0
test_tables "universal_asset_comma_16LE_with_BOM.csv" 48 "TAB_16LE_BOM"
#curlfail_pop
print_result $REZ

if false;then
echo "********* asset_import_err.sh *********************************************************************"
echo "********* 2. Method is not allowed ****************************************************************"
echo "***************************************************************************************************"
test_it "Method_is_not_allowed"
curlfail_push_expect_400
initiate_db
REZ=0
ASSET="$CHECKOUTDIR/tools/asset_import/very_long_asset_tab_16LE.csv"
api_auth_get /asset/import assets=@$ASSET -H "Expect:"&& echo "$OUT_CURL" | $JSONSH -N  >&5
REZ=$?
curlfail_pop
print_result $REZ
fi
if false;then
echo "********* asset_import_err.sh *********************************************************************"
echo "********* 2. Export universal tab 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_tab_16LE"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_tab_16LE.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_tab_16LE.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_tab_16LE.csv
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 3. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_tab_16LE.csv" 48 "TAB_16LE"
#echo "REZ5=$REZ"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 4. Import universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_tab_8"
initiate_db
REZ=0
test_tables "universal_asset_tab_8.csv" 48 "TAB_8"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 5. Export universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_tab_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_tab_8.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_tab_8.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_tab_8.csv
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 6. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_tab_8.csv" 48 "_8"
#echo "REZ6=$REZ"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 7. Import universal comma 16LE **********************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_comma_16LE"
initiate_db
REZ=0
test_tables "universal_asset_comma_16LE.csv" 48 "COMMA_16LE"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 8. Export universal comma 16LE **********************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_16LE"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_comma_16LE.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_comma_16LE.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_comma_16LE.csv
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 9. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_comma_16LE.csv" 48 "COMMA_16LE"
#echo "REZ5=$REZ"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 10. Import universal comma 8 ************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_comma_8"
initiate_db
REZ=0
test_tables "universal_asset_comma_8.csv" 48 "COMMA_8"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 11. Export universal comma 8 ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_comma_8.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_comma_8.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_comma_8.csv
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 12. Import exported file without ids ****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_comma_8.csv" 48 "COMMA_8"
#echo "REZ6=$REZ"
print_result $REZ
fi
