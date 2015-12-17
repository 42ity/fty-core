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
#! \file   asset_import.sh
#  \brief  CI tests for asset import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

echo
echo "###################################################################################################"
echo "********* asset_import.sh ************************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
[ -n "$CSV_LOADDIR_ASSIMP" ] && [ -d "$CSV_LOADDIR_ASSIMP" ] || die "Can not use CSV_LOADDIR_ASSIMP='$CSV_LOADDIR_ASSIMP'"

res_err(){
RESULT=$1
DESCR=$2
if [ ${RESULT} != 0 ];then
    echo "ERROR : ${DESCR}"
fi
}

table_diff(){
TABLE_NAME="$1"
TEST_ID="$2"
RES_PART=0
# dump requested table and compare it with expected content
sut_run "mysqldump -u root box_utf8 ${TABLE_NAME}" |grep "INSERT" > "${DB_DUMP_DIR}/${TABLE_NAME}.dmp"
#if [ "z${TEST_ID}" != "z" ] && [ -f "${DB_RES_DIR}/${TABLE_NAME}${TEST_ID}.ptr" ] ; then
if [ -f "${DB_RES_DIR}/${TABLE_NAME}${TEST_ID}.ptr" ] ; then
    diff "${DB_DUMP_DIR}/${TABLE_NAME}.dmp" "${DB_RES_DIR}/${TABLE_NAME}${TEST_ID}.ptr" > /dev/null || RES_PART=1
else
    diff "${DB_DUMP_DIR}/${TABLE_NAME}.dmp" "${DB_RES_DIR}/${TABLE_NAME}.ptr" > /dev/null || RES_PART=1
fi
RES=$(expr ${RES} + ${RES_PART})
}

csv_import(){
CSV_FILE_NAME="$1"
TEST_ID="$2"
# import csv with assets
ASSET="${CSV_LOADDIR_ASSIMP}/${CSV_FILE_NAME}"
api_auth_post_file_form_json /asset/import assets="@$ASSET" >&5
RES_PART=$?
res_err "$RES_PART" "The import of $CSV_FILE_NAME was not successful"
# tables compared with patterns using dif using function table_diff <table name> <test id>
# pattern file full names are : ${DB_RES_DIR}/${TABLE_NAME}${TEST_ID}.ptr
for i in "t_bios_asset_element" "t_bios_asset_group_relation" "t_bios_asset_ext_attributes" "t_bios_asset_link" "t_bios_asset_link_type"; do
    table_diff "$i" "$TEST_ID"
    res_err "$RES_PART" "Wrong $i table content."
    NUM_EXPECTED=0
done
}

echo "********* asset_import.sh *************************************************************************"
echo "********* 1. Import universal tab 16LE ************************************************************"
echo "***************************************************************************************************"
test_it "Import_universal_tab_16LE"
loaddb_initial
RES=0
# The file is imported, dumped tables and compared with pattern files
csv_import "universal_asset_tab_16LE.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 2. Export universal tab 16LE ************************************************************"
echo "***************************************************************************************************"
# DB asset tables content created in the tc "Import_universal_tab_16LE" is exported to exp_uni_tab_16LE.csv
# Before the writting to the file is filtered http header
test_it "Export_universal_tab_16LE"
#*#*#*#*#*#*# TODO : Export_universal_tab_16LE : does it really true we get the http header to the exported file?
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > ${CSV_LOADDIR_ASSIMP}/exp_uni_tab_16LE.csv
RES=$?
# The last item from the exported file - id - is removed
sed 's/\(,[^,]*\)$//' ${CSV_LOADDIR_ASSIMP}/exp_uni_tab_16LE.csv > ${CSV_LOADDIR_ASSIMP}/imp_exp_uni_tab_16LE.csv
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 3. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
# Exported and filtered file "imp_exp_uni_tab_16LE.csv" is imported
# the dumped tables are compared with original pattern files.
test_it "Import_exported_file_without_ids"
loaddb_initial
RES=0
csv_import "imp_exp_uni_tab_16LE.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 4. Import universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
# the same like in previous tc's used tab delimiter UTF-8 encoding csv source file
test_it "Import_universal_tab_8"
loaddb_initial
RES=0
csv_import "universal_asset_tab_8.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 5. Export universal tab 8 ***************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_tab_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CSV_LOADDIR_ASSIMP/exp_uni_tab_8.csv
RES=$?
sed 's/\(,[^,]*\)$//' $CSV_LOADDIR_ASSIMP/exp_uni_tab_8.csv > $CSV_LOADDIR_ASSIMP/imp_exp_uni_tab_8.csv
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 6. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
loaddb_initial
RES=0
csv_import "imp_exp_uni_tab_8.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 7. Import universal comma 16LE **********************************************************"
echo "***************************************************************************************************"
# the same like in previous tc's used comma delimiter UTF-16LE encoding csv source file
test_it "Import_universal_comma_16LE"
loaddb_initial
RES=0
csv_import "universal_asset_comma_16LE.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 8. Export universal comma 16LE **********************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_16LE"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > $CSV_LOADDIR_ASSIMP/exp_uni_comma_16LE.csv
RES=$?
sed 's/\(,[^,]*\)$//' $CSV_LOADDIR_ASSIMP/exp_uni_comma_16LE.csv > $CSV_LOADDIR_ASSIMP/imp_exp_uni_comma_16LE.csv
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 9. Import exported file without ids *****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
loaddb_initial
RES=0
csv_import "imp_exp_uni_comma_16LE.csv"
#echo "RES5=$RES"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 10. Import universal comma 8 ************************************************************"
echo "***************************************************************************************************"
# the same like in previous tc's used comma delimiter UTF-8 encoding csv source file
test_it "Import_universal_comma_8"
loaddb_initial
RES=0
csv_import "universal_asset_comma_8.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 11. Export universal comma 8 ************************************************************"
echo "***************************************************************************************************"
test_it "Export_universal_comma_8"
api_auth_get /asset/export -H "Expect:" | grep -v '>'|grep -v '<'|grep -v '*'|grep -v '{' | awk 'NF' > ${CSV_LOADDIR_ASSIMP}/exp_uni_comma_8.csv
RES=$?
sed 's/\(,[^,]*\)$//' $CSV_LOADDIR_ASSIMP/exp_uni_comma_8.csv > $CSV_LOADDIR_ASSIMP/imp_exp_uni_comma_8.csv
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 12. Import exported file without ids ****************************************************"
echo "***************************************************************************************************"
test_it "Import_exported_file_without_ids"
loaddb_initial
RES=0
csv_import "imp_exp_uni_comma_8.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 13. Import universal asset comma 16LE with BOM ******************************************"
echo "***************************************************************************************************"
# the same like in previous tc's used comma delimiter UTF-16LE encoding csv source file with BOM
test_it "universal_asset_comma_16LE_with_BOM"
loaddb_initial
RES=0
csv_import "universal_asset_comma_16LE_with_BOM.csv"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 14. Case insensitive ********************************************************************"
echo "***************************************************************************************************"
# test of case insensivity off headers and some values inside csv file 
test_it "Case_insensitive"
loaddb_initial
RES=0
csv_import "universal_asset_comma_insensitive_8.csv" "_case_insensitive"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 15. Asset link **************************************************************************"
echo "***************************************************************************************************"
# test of the all links possible in t_bios_asset_link table
test_it "Asset link"
loaddb_initial
RES=0
csv_import "universal_asset_comma_8_asset_link.csv" "_asset_link"
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 16. Export universal tab 16LE with header ***********************************************"
echo "***************************************************************************************************"
# Content-Disposition in the export header
test_it "Export_universal_tab_16LE"
api_auth_get /asset/export -H "Expect:"|grep 'Content-Disposition'|grep 'asset_export201[0-9]-[0-1][0-9]-[0-3][0-9].csv'
RES=$?
print_result $RES

echo "********* asset_import.sh *************************************************************************"
echo "********* 17. Export id *************************** ***********************************************"
echo "***************************************************************************************************"
# exported id present and proper in exported file.
test_it "Export_id"
diff "${CSV_LOADDIR_ASSIMP}/exp_uni_tab_16LE.csv" "${DB_RES_DIR}/exp_uni_tab_16LE.ptr" || RES=1
print_result $RES

echo
echo "###################################################################################################"
echo "********* asset_import.sh *************************** END *****************************************"
echo "###################################################################################################"
echo
