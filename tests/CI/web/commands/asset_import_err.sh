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

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

_SCRIPT_NAME="asset_import"
# Add the library
#. $CHECKOUTDIR/tests/CI/scriptlib.sh || \
#    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
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
echo "********* asset_import_err.sh ********************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 1. No utf csv file **********************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*#*# TODO : No utf csv file : Really should PASS not allowed format?
test_it "No_utf_csv_file"
initiate_db
REZ=0
test_tables "universal_asset_comma_no_utf.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 2. Method is not allowed ****************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*#*# TODO : Method is not allowed : Wrong message : MUST BE LIKE : Method is not allowed : 45
test_it "Method_is_not_allowed"
curlfail_push_expect_400
initiate_db
REZ=0
ASSET="$CHECKOUTDIR/tools/asset_import/universal_asset_semicolon_16LE.csv"
api_auth_get /asset/import assets=@$ASSET -H "Expect:"&& echo "$OUT_CURL" | $JSONSH -N  >&5
REZ=$?
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 3. Too big file *************************************************************************"
echo "***************************************************************************************************"
test_it "Too_big_file"
curlfail_push_expect_413
initiate_db
REZ=0
test_tables "very_long_asset_tab_16LE.csv" 48 "ERROR"
curlfail_pop
print_result $REZ



echo "********* asset_import_err.sh *********************************************************************"
echo "********* 4. File assets is missing ***************************************************************"
echo "***************************************************************************************************"
test_it "File_assets_is_missing"
DATASWITCH=1
curlfail_push_expect_400
initiate_db
REZ=0
test_tables "missing_file.csv" 48 "ERROR"
curlfail_pop
DATASWITCH=0
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 5. Missing mandatory name header ********************************************************"
echo "***************************************************************************************************"
test_it "Missing_mandatory_name_header"
curlfail_push_expect_400
initiate_db
REZ=0
test_tables "universal_asset_tab_8_no_name_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_type_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_sub_type_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_location_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_status_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_business_critical_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_priority_header.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 6. Bad separator ************************************************************************"
echo "***************************************************************************************************"
test_it "Too_big_file"
#*#*#*#*#*#*# TODO : Bad separator : the code 48 received is not expected : 47???
curlfail_push_expect_400
initiate_db
REZ=0
test_tables "universal_asset_comma_16LE_bad_separator.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 7. 16BE - BIG ENDIAN ********************************************************************"
echo "***************************************************************************************************"
test_it "16BE_-_BIG_ENDIAN"
curlfail_push_expect_400
initiate_db
REZ=0
test_tables "universal_asset_comma_16BE.csv" 48 "ERROR"
test_tables "universal_asset_semicolon_16BE.csv" 48 "ERROR"
test_tables "universal_asset_tab_16BE.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 8. Not so long **************************************************************************"
echo "***************************************************************************************************"
test_it "Not_so_long"
#*#*#*#*#*#*# TODO : Not so long : 128kB is 131072 byte : the not_so_long_asset_tab_16LE.csv has size 130860
curlfail_push_expect_413
initiate_db
REZ=0
test_tables "not_so_long_asset_tab_16LE.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 8. Too long keytag **********************************************************************"
echo "***************************************************************************************************"
test_it "Too_long_keytag"
#*#*#*#*#*#*# TODO : Too_long_keytag : 128kB is 131072 byte : the not_so_long_asset_tab_16LE.csv has size 130860
initiate_db
REZ=0
test_tables "universal_asset_tab_8_too_long_keytag.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 9. Not proper sequence ******************************************************************"
echo "***************************************************************************************************"
test_it "Not_proper_sequence"
initiate_db
REZ=0
test_tables "universal_asset_tab_16LE_DC001A.csv" 47 "ERROR" "_not_proper_sequence"
#csv_import "$CSV_FILE_NAME" "t_bios_asset_element" 0 "$FILENAME_PREFIX"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 10. Double records **********************************************************************"
echo "***************************************************************************************************"
test_it "Double_records"
initiate_db
REZ=0
test_tables "universal_asset_tab_16LE_DC001B.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 11. Comma semicolon mix *****************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*# TODO : Comma semicolon mix : should be stopped and give some message?
test_it "Comma_semicolon_mix"
initiate_db
REZ=0
test_tables "universal_asset_mix_comma_semicolon__8.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 12. Tab comma mix ***********************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*# TODO : Tab comma mix : should be stopped and give some message?
test_it "Tab_comma_mix"
initiate_db
REZ=0
test_tables "universal_asset_mix_tab_comma_8.csv" 48 "ERROR"
print_result $REZ

