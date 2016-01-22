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
#! \file   asset_import_err.sh
#  \brief  CI tests for errors in asset import
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

echo
echo "###################################################################################################"
echo "********* asset_import_err.sh ********************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
[ -n "$CSV_LOADDIR_ASSIMP" ] && [ -d "$CSV_LOADDIR_ASSIMP" ] || die "Can not use CSV_LOADDIR_ASSIMP='$CSV_LOADDIR_ASSIMP'"
#init_script

DATASWITCH=0

csv_import_err(){
    CSV_FILE_NAME="$1"
    TABLE_NAME="$2"
    NUM_EXPECTED="$3" # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
    FILENAME_PREFIX="$4"
    ASSET="$CSV_LOADDIR_ASSIMP/${CSV_FILE_NAME}"
    if [ "$NUM_EXPECTED" != 0 ]; then
        if [ "$REZ" = 0 ]; then
            if [ "$DATASWITCH" == 0 ]; then
                api_auth_post_file_form_json /asset/import assets="@$ASSET" >&5
                REZ=$?
            else
                api_auth_post_file_data_json /asset/import assets="@$ASSET" >&5
                REZ=$?
            fi
            echo "REZ = $REZ"
        fi
    fi
}

test_tables(){
    CSV_FILE_NAME="$1"
    NUM_EXPECTED="$2" # when NUM_EXPECTED=0, the import is not performed, in other case it is num of imported lines
    FILENAME_PREFIX="$3"
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

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 1. No utf csv file **********************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*#*# TODO : No utf csv file : Really should PASS not allowed format? SOLVED!
test_it "No_utf_csv_file"
loaddb_initial
REZ=0
test_tables "universal_asset_comma_no_utf.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 2. Method is not allowed ****************************************************************"
echo "***************************************************************************************************"
test_it "Method_is_not_allowed"
curlfail_push_expect_405
loaddb_initial
REZ=0
ASSET="$CSV_LOADDIR_ASSIMP/universal_asset_semicolon_16LE.csv"
# Params do not really matter as this should fail
api_auth_get_json /asset/import assets="@$ASSET" -H "Expect:" >&5
REZ=$?
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 3. Too big file *************************************************************************"
echo "***************************************************************************************************"
test_it "Too_big_file"
curlfail_push_expect_413
loaddb_initial
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
loaddb_initial
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
loaddb_initial
REZ=0
test_tables "universal_asset_tab_8_no_name_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_type_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_sub_type_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_location_header.csv" 48 "ERROR"
test_tables "universal_asset_tab_8_no_status_header.csv" 48 "ERROR"
#test_tables "universal_asset_tab_8_no_priority_header.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 6. Bad separator ************************************************************************"
echo "***************************************************************************************************"
test_it "Bad_separator"
#*#*#*#*#*#*# TODO : Bad separator : the code 48 received is not expected : 47??? SOLVED!! 48 - OK
curlfail_push_expect_400
loaddb_initial
REZ=0
test_tables "universal_asset_comma_16LE_bad_separator.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 7. 16BE - BIG ENDIAN ********************************************************************"
echo "***************************************************************************************************"
test_it "16BE_-_BIG_ENDIAN"
curlfail_push_expect_400
loaddb_initial
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
#*#*#*#*#*#*# TODO : Not so long : 128kB is 131072 byte : the not_so_long_asset_tab_16LE.csv has size 130860: Is planned!!!
curlfail_push_expect_413
loaddb_initial
REZ=0
test_tables "not_so_long_asset_tab_16LE.csv" 48 "ERROR"
curlfail_pop
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 9. Too long keytag **********************************************************************"
echo "***************************************************************************************************"
if [ x"$TEST_BIOS_1516" = xyes ] ; then
test_it "Too_long_keytag"
loaddb_initial
REZ=0
test_tables "universal_asset_tab_8_too_long_keytag.csv" 48 "ERROR"
# Expected output:
# {"imported_lines":42,"errors":[[20,"Internal Server Error. "],[28,"Element 'UPS1' not found."],[29,"Element 'ePDU1' not found."],[35,"Element 'ePDU2' not found."],[36,"Element 'ePDU2' not found."],[39,"Element 'ePDU2' not found."]]}
# In practice it flip-flops so only line20 fails, so the test is skipped for now
print_result $REZ
else logmsg_warn "TEST SKIPPED (see BIOS-1516)"; fi

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 10. Not proper sequence *****************************************************************"
echo "***************************************************************************************************"
test_it "Not_proper_sequence"
loaddb_initial
REZ=0
test_tables "universal_asset_tab_16LE_DC001A.csv" 47 "ERROR" "_not_proper_sequence"
#csv_import "$CSV_FILE_NAME" "t_bios_asset_element" 0 "$FILENAME_PREFIX"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 11. Double records **********************************************************************"
echo "***************************************************************************************************"
test_it "Double_records"
loaddb_initial
REZ=0
test_tables "universal_asset_tab_16LE_DC001B.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 12. Comma semicolon mix *****************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*# TODO : Comma semicolon mix : should be stopped and give some message?
test_it "Comma_semicolon_mix"
loaddb_initial
REZ=0
test_tables "universal_asset_mix_comma_semicolon__8.csv" 48 "ERROR"
print_result $REZ


echo "********* asset_import_err.sh *********************************************************************"
echo "********* 13. Tab comma mix ***********************************************************************"
echo "***************************************************************************************************"
#*#*#*#*#*# TODO : Tab comma mix : should be stopped and give some message?
test_it "Tab_comma_mix"
loaddb_initial
REZ=0
test_tables "universal_asset_mix_tab_comma_8.csv" 48 "ERROR"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 14. Wrong maximum number of racks *******************************************************"
echo "***************************************************************************************************"
test_it "Wrong_maximum_number_of_racks"
#*#*#*#*#*#*# TODO : Wrong_maximum_number_of_racks : allow 10q and from u10 makes 10
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_max_num_rack_wrong_8.csv" 48 "ERROR" "_max_num_rack"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 15. Wrong u_size ************************************************************************"
echo "***************************************************************************************************"
test_it "Wrong_u_size"
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_u_size_wrong_8.csv" 48 "ERROR" "_u_size"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 16. Runtime *****************************************************************************"
echo "***************************************************************************************************"
test_it "Runtime"
#*#*#*#*#*#*# TODO : Runtime : for sub_type non-genset MUST be ommited, must be integer, both is not
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_runtime_8.csv" 48 "ERROR" "_runtime"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 17. Phase *******************************************************************************"
echo "***************************************************************************************************"
test_it "Phase"
#*#*#*#*#*#*# TODO : Phase : for sub_type non-feed MUST be ommited, must be 1,2 or 3, both is not
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_phase_8.csv" 48 "ERROR" "_phase"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 18. Email *******************************************************************************"
echo "***************************************************************************************************"
test_it "Email"
#*#*#*#*#*#*# TODO : Email : When the mail address (or any CSV content at the moment) contains " or ', Internal error is returned, the ` are allowed, addr itself is NOT checked for "email validity"
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_email_8.csv" 48 "ERROR" "_email"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 19. Not consequently ********************************************************************"
echo "***************************************************************************************************"
test_it "Not_consequently"
loaddb_initial
REZ=0
test_tables "universal_asset_semicolon_not_consequently_8.csv" 48 "ERROR" "_phase"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 20. Double name 1 ***********************************************************************"
echo "***************************************************************************************************"
test_it "Double_name_1"
loaddb_initial
REZ=0
curlfail_push_expect_500
test_tables "universal_asset_comma_8_double_name_1.csv" 48 "ERROR" "_double_name_1"
print_result $REZ
curlfail_pop

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 21. Double name 2 ***********************************************************************"
echo "***************************************************************************************************"
test_it "Double_name_2"
loaddb_initial
REZ=0
curlfail_push_expect_500
test_tables "universal_asset_comma_8_double_name_2.csv" 48 "ERROR" "_double_name_2"
print_result $REZ
curlfail_pop

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 22. Duplicate name value ****************************************************************"
echo "***************************************************************************************************"
test_it "Duplicate_name_value"
#*#*#*#*#*#*# TODO : Duplicate_name_value : There should not be code 200 OK
loaddb_initial
REZ=0
ASSET="$CSV_LOADDIR_ASSIMP/universal_asset_comma_8.csv"
# Not JSON, not to >&5 :
api_auth_post_file_form /asset/import assets="@$ASSET"
ASSET="$CSV_LOADDIR_ASSIMP/universal_asset_comma_8_duplicate_name_value.csv"
api_auth_post_file_form_json /asset/import assets="@$ASSET" >&5
print_result $?

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 23. Duplicate ipx ***********************************************************************"
echo "***************************************************************************************************"
test_it "Duplicate_ipx"
#*#*#*#*#*#*# TODO : Duplicate_ipx : is it allowed?
loaddb_initial
REZ=0
test_tables "universal_asset_comma_8_duplicate_ipx.csv" 48 "ERROR" "_duplicate_ipx"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 24. w_pos *******************************************************************************"
echo "***************************************************************************************************"
test_it "w_pos"
loaddb_initial
REZ=0
test_tables "universal_asset_comma_8_wpos.csv" 48 "ERROR" "_w_pos"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 25. Not unique serial No ****************************************************************"
echo "***************************************************************************************************"
test_it "Not_unique_serial_No"
loaddb_initial
REZ=0
test_tables "universal_asset_comma_8_serial_no.csv" 48 "ERROR" "_serial_no"
print_result $REZ

echo "********* asset_import_err.sh *********************************************************************"
echo "********* 26. Subtype_&_status_values *************************************************************"
echo "***************************************************************************************************"
test_it "Subtype & status values"
loaddb_initial
REZ=0
test_tables "universal_asset_comma_8_subtype_status_values.csv" 48 "ERROR" "_subtype_status_values"
print_result $REZ

echo "********* asset_import_err.sh *************************************************************************"
echo "********* 27. Asset tag values *************************************************************"
echo "***************************************************************************************************"
test_it "Asset_tag_values"
#*#*#*#*#*#*# TODO : error msg contais "to long", should "too long"
loaddb_initial
REZ=0
test_tables "universal_asset_comma_8_asset_tag.csv" 48 "ERROR" "_asset_tag"
print_result $REZ

echo "###################################################################################################"
echo "********* asset_import_err.sh *********************** END *****************************************"
echo "###################################################################################################"
echo
