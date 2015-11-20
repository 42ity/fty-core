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
# with this program; in case not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   asset_import.sh
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

csv_import(){
CSV_FILE_NAME=$1
TABLE_NAME=$2
NUM_EXPECTED=$3 # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
FILENAME_PREFIX=$4
TEST_ID=$5
ASSET="$CHECKOUTDIR/tools/asset_import/${CSV_FILE_NAME}"
if [ "$NUM_EXPECTED" != 0 ];then
    if [ "$REZ" = 0 ];then
        api_auth_post_file /asset/import assets=@$ASSET -H "Expect:" && echo "$OUT_CURL" | $JSONSH -N  >&5
        REZ=$?
#sleep 120
#        echo "REZ = $REZ"
    fi
fi
mysqldump -u root box_utf8 ${TABLE_NAME}|grep "INSERT" > ${DUMP_DIR}/${TABLE_NAME}.dmp
if [ "$REZ" = 0 ];then
    diff ${DUMP_DIR}/${TABLE_NAME}.dmp ${RES_DIR}/${TABLE_NAME}${TEST_ID}.ptr|wc -l || REZ=1
fi
}

test_tables(){
CSV_FILE_NAME=$1
NUM_EXPECTED=$2 # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
FILENAME_PREFIX=$3
TEST_ID=$4
csv_import "$CSV_FILE_NAME" "t_bios_asset_element" "$NUM_EXPECTED" "$FILENAME_PREFIX" "$TEST_ID"
echo "REZ1=$REZ"
csv_import "$CSV_FILE_NAME" "t_bios_asset_group_relation" 0 "$FILENAME_PREFIX" "$TEST_ID"
echo "REZ2=$REZ"
csv_import "$CSV_FILE_NAME" "t_bios_asset_ext_attributes" 0 "$FILENAME_PREFIX" "$TEST_ID"
echo "REZ3=$REZ"
csv_import "$CSV_FILE_NAME" "t_bios_asset_link" 0 "$FILENAME_PREFIX" "$TEST_ID"
echo "REZ4=$REZ"
csv_import "$CSV_FILE_NAME" "t_bios_asset_link_type" 0 "$FILENAME_PREFIX" "$TEST_ID"
echo "REZ4a=$REZ"
}

echo
echo "###################################################################################################"
echo "********* asset_import.sh ************************** START ****************************************"
echo "###################################################################################################"
echo

if false;then

echo "********* asset_import.sh *************************************************************************"
echo "********* 1. Import universal tab 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_tab_16LE"
initiate_db
REZ=0
test_tables "universal_asset_tab_16LE.csv" 48 "TAB_16LE"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 2. Export universal tab 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_tab_16LE"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_tab_16LE.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_tab_16LE.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_tab_16LE.csv
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 3. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_tab_16LE.csv" 48 "TAB_16LE"
#echo "REZ5=$REZ"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 4. Import universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_tab_8"
initiate_db
REZ=0
test_tables "universal_asset_tab_8.csv" 48 "TAB_8"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 5. Export universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_tab_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_tab_8.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_tab_8.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_tab_8.csv
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 6. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_tab_8.csv" 48 "_8"
#echo "REZ6=$REZ"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 7. Import universal comma 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_comma_16LE"
initiate_db
REZ=0
test_tables "universal_asset_comma_16LE.csv" 48 "COMMA_16LE"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 8. Export universal comma 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_16LE"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_comma_16LE.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_comma_16LE.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_comma_16LE.csv
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 9. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_comma_16LE.csv" 48 "COMMA_16LE"
#echo "REZ5=$REZ"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 10. Import universal comma 8 ************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_comma_8"
initiate_db
REZ=0
test_tables "universal_asset_comma_8.csv" 48 "COMMA_8"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 11. Export universal comma 8 ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CHECKOUTDIR/tools/asset_import/exp_uni_comma_8.csv
REZ=$?
sed 's/\(,[^,]*\)$//' $CHECKOUTDIR/tools/asset_import/exp_uni_comma_8.csv > $CHECKOUTDIR/tools/asset_import/imp_exp_uni_comma_8.csv
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 12. Import exported file without ids ****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
initiate_db
REZ=0
test_tables "imp_exp_uni_comma_8.csv" 48 "COMMA_8"
#echo "REZ6=$REZ"
print_result $REZ

echo "********* asset_import.sh *************************************************************************"
echo "********* 13. Import universal asset comma 16LE with BOM ****************************************************"
echo "***************************************************************************************************"
test_it "universal_asset_comma_16LE_with_BOM"
initiate_db
REZ=0
test_tables "universal_asset_comma_16LE_with_BOM.csv" 48 "COMMA_8"
#echo "REZ6=$REZ"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 14. Case insensitive ********************************************************************"
echo "***************************************************************************************************"
test_it "Case_insensitive"
initiate_db
REZ=0
test_tables "universal_asset_comma_insensitive_8.csv" 48 "ERROR" "_case_insensitive"
print_result $REZ

fi

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 15. Wrong maximum number of racks *******************************************************"
echo "***************************************************************************************************"
test_it "Wrong_maximum_number_of_racks"
#*#*#*#*#*#*# TODO : allow 10q and from u10 makes 10
initiate_db
REZ=0
test_tables "universal_asset_semicolon_max_num_rack_wrong_8.csv" 48 "ERROR" "_max_num_rack"
print_result $REZ
