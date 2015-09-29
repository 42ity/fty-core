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
#! \file   asset_create.sh
#  \brief  CI tests for asset create and delete calls
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>

echo "***************************************************************************************************"
echo "WEBLIB_CURLFAIL = $WEBLIB_CURLFAIL"
curlfail_push_expect_400
test_it "asset_no_content"
api_auth_post /asset '' >/dev/null
PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x message`
echo "${PARSED_REPLY}" | cut -d '"' -f 6 | grep -q -E 'Error deserializing POST request document' 
if [ $? -eq 0 ]; then
  PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x code`
  echo "${PARSED_REPLY}" | cut -d '"' -f 5 | grep -q -E ']	0'
fi
print_result $?
curlfail_pop

echo "***************************************************************************************************"
curlfail_push_expect_400
test_it "asset_no_content2"
api_auth_post /asset '{}' >/dev/null

PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x message`
echo "${PARSED_REPLY}" | cut -d '"' -f 6 | grep -q -E "column 'name' is missing, import is aborted"
if [ $? -eq 0 ]; then
  PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x code`
  echo "${PARSED_REPLY}" | cut -d '"' -f 5 | grep -q -E ']	0'
fi
print_result $?
curlfail_pop

echo "***************************************************************************************************"
test_it "DC asset_create"
api_auth_post /asset '{"name":"dc_name_test","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B123","address":"ASDF"}}' >/dev/null
PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x id`
ID_DC=`echo "${PARSED_REPLY}" | cut -d '"' -f 4`
echo "${PARSED_REPLY}" | cut -d '"' -f 4 | grep -q -E '^[0-9]+$'
print_result $?

echo "***************************************************************************************************"
test_it "FEED asset_create"
api_auth_post /asset '{"name":"FEED1","type":"device","sub_type":"feed","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B124","address":"ASDF"}}' >/dev/null
#test the reply
echo "${PARSED_REPLY}" | cut -d '"' -f 4 | grep -q -E '^[0-9]+$'
print_result $?

echo "***************************************************************************************************"
curlfail_push_expect_400
test_it "ePDU asset create error"
api_auth_post /asset '{"name":"ePDU1","type":"device","sub_type":"epdu","location":"dc_name_test","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B125","address":"ASDF"}}' >/dev/null
PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x message`
echo "${PARSED_REPLY}" | cut -d '"' -f 6 | grep -q -E "Need to specify attribute location_w_pos for"
if [ $? -eq 0 ]; then
  PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x code`
  echo "${PARSED_REPLY}" | cut -d '"' -f 5 | grep -q -E ']	70'
fi
print_result $?
curlfail_pop

echo "***************************************************************************************************"
curlfail_push_expect_400
test_it "asset_create_dup"
api_auth_post /asset '{"name":"dc_name_test","type":"datacenter","sub_type":"","location":"","status":"active","business_critical":"yes","priority":"P1","ext":{"asset_tag":"A123B123","address":"ASDF"}}' >/dev/null
print_result $?
curlfail_pop

echo "***************************************************************************************************"
# remove the asset - it'll fail if previous attempt will fail too
curlfail_push_expect_4xx5xx
#test_it "asset_delete_previously_inserted"
ID=$ID_DC
api_auth_delete /asset/${ID} >/dev/null

PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x message`
echo PARSED_REPLY = "${PARSED_REPLY}" || echo "${OUT_CURL}"
#PARSED_REPLY2 = echo "${PARSED_REPLY}" | cut -d '"' -f 6|grep -v "!"
echo "${PARSED_REPLY}" | grep -q -E "cannot be processed because of conflict. Asset has elements inside, DELETE them first"
#echo "${PARSED_REPLY}" | cut -d '"' -f 6 | grep -q -E "cannot be processed because of conflict. Asset has elements inside, DELETE them first"
echo retur = $?
if [ $? -eq 0 ]; then
  PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x code` || echo "${OUT_CURL}"
  echo "${PARSED_REPLY}" | cut -d '"' -f 5 | grep -q -E ']	58'
fi
print_result $?
curlfail_pop
echo "***************************************************************************************************"
if [ 1 -eq 0 ];then 
test_it "asset_delete_previously_deleted"
if [ $RES1 -eq 0 ];then
  curlfail_push_expect_404
  api_auth_delete /asset/${ID} >/dev/null
  PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x message`
  echo "${PARSED_REPLY}" | cut -d '"' -f 6 | grep -q -E "Asset doesn't exist"
  if [ $? -eq 0 ]; then
    PARSED_REPLY=`echo ${OUT_CURL} | $JSONSH -x code`
    echo "${PARSED_REPLY}" | cut -d '"' -f 5 | grep -q -E ']	57'
    RES2=$?
  fi
  curlfail_pop
else
  RES2=$RES1
fi
print_result $RES2
fi
