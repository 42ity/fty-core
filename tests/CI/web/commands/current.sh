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
#! \file   current.sh
#  \brief  CI tests for asset current
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

echo
echo "###################################################################################################"
echo "********* current.sh ******************************* START ****************************************"
echo "###################################################################################################"
echo

# Add the first line of the sql file and create it
DB_TMPSQL_FILE_CURRENT="${DB_TMPSQL_DIR}/tmp-current-$$.sql"

echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"

# Function definition - Add the measurement chosen from the SAMPLES variable
measurement() {
    UNIT="$1"
    TOPIC="$2"
    DEVICE="$3"
    TIMESTAMP="$4"
    VALUE="$5"
    SCALE="$6"

    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
    SLCT="select id from t_bios_measurement_topic where topic='$TOPIC'"
    TOP_ID="`do_select "$SLCT"`"
    if [ "$TOP_ID" == "" ]; then
        sqlline="INSERT INTO t_bios_measurement_topic (device_id, units,topic) SELECT r.id_discovered_device,'$UNIT','$TOPIC' FROM t_bios_asset_element AS e,t_bios_monitor_asset_relation AS r WHERE e.name = '$DEVICE' AND e.id_asset_element = r.id_asset_element;"
        echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
        sqlline="set @topic_id = LAST_INSERT_ID();"
    else
        sqlline="set @topic_id = $TOP_ID;"
    fi
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    # measurement
    sqlline="INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ($TIMESTAMP, $VALUE, $SCALE, @topic_id);"
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file ""${DB_TMPSQL_FILE_CURRENT}""
    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
}

db_initiate(){
    LOADDB_FILE_REMOTE_SLEEP=5 loaddb_current || return $?

    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
    sqlline="INSERT INTO t_bios_discovered_device (id_discovered_device,name,id_device_type) VALUES (NULL, 'DC-LAB', 1);"
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file ""${DB_TMPSQL_FILE_CURRENT}""
    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
    SEL_ID="select id_discovered_device from t_bios_discovered_device where name='DC-LAB'"
    TOP_ID="$(do_select "$SEL_ID")"
    sqlline="INSERT INTO t_bios_monitor_asset_relation (id_ma_relation,id_discovered_device,id_asset_element) VALUES (NULL, $TOP_ID, 19);"
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file ""${DB_TMPSQL_FILE_CURRENT}""
}

db_measure(){
    # set actual time
    TIME=$(date +%s)
    STEP="$1"
    VAL="$3"
    D0="$2"
    D1=$(expr $D0 - $STEP)
    D2=$(expr $D1 - $STEP)
    D3=$(expr $D2 - $STEP)
    D4=$(expr $D3 - $STEP)
    D5=$(expr $D4 - $STEP)
    D6=$(expr $D5 - $STEP)
    D7=$(expr $D6 - $STEP)
    D8=$(expr $D7 - $STEP)
    D9=$(expr $D8 - $STEP)
    D10=$(expr $D9 - $STEP)
    D11=$(expr $D10 - $STEP)
    D12=$(expr $D11 - $STEP)
    D13=$(expr $D12 - $STEP)
    D14=$(expr $D13 - $STEP)
    D15=$(expr $D14 - $STEP)
    D16=$(expr $D15 - $STEP)
    D17=$(expr $D16 - $STEP)
    D18=$(expr $D17 - $STEP)
    D19=$(expr $D18 - $STEP)

    # set the header names of the params
    PARAMS=(units topic name timestamp value scale)

    #Define the measurements samples
    SAMPLES=(
NULL    status.ups@UPS1-LAB     	UPS1-LAB    	$(expr $TIME - $D0)     $(expr 400 + $VAL)    -2
%       load.default@UPS1-LAB   	UPS1-LAB        $(expr $TIME - $D0)     $(expr 435 + $VAL)    -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D1)     $(expr 470 + $VAL)    -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D2)     $(expr 475 + $VAL)    -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D3)     $(expr 478 + $VAL)    -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D4)     $(expr 473 + $VAL)    -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D5)     $(expr 471 + $VAL)    -2
W       realpower.output.L1@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D6)     $(expr 505 + $VAL)    -2
A       current.output.L1@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D7)     $(expr 540 + $VAL)    -2
%       charge.battery@UPS1-LAB 	UPS1-LAB        $(expr $TIME - $D8)     $(expr 575 + $VAL)    -2
s       runtime.battery@UPS1-LAB        UPS1-LAB        $(expr $TIME - $D9)     $(expr 610 + $VAL)    -2
W       realpower.default@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D10)    $(expr 645 + $VAL)    -2
V       voltage.output.L2-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D11)    $(expr 680 + $VAL)    -2
W       realpower.output.L2@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D12)    $(expr 715 + $VAL)    -2
A       current.output.L2@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D13)    $(expr 750 + $VAL)    -2
V       voltage.output.L3-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D14)    $(expr 785 + $VAL)    -2
W       realpower.output.L3@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D15)    $(expr 820 + $VAL)    -2
A       current.output.L3@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D16)    $(expr 855 + $VAL)    -2
A       current.output.L8@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D17)    $(expr 890 + $VAL)    -2
W       nonsense@UPS1-LAB       	UPS1-LAB        $(expr $TIME - $D18)    $(expr 925 + $VAL)    -2
W       realpower.outlet.1@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D0)     $(expr 960 + $VAL)    -2
W       realpower.outlet.2@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D1)     $(expr 995 + $VAL)    -2
W       realpower.outlet.15@ePDU1-LAB   ePDU1-LAB       $(expr $TIME - $D2)     $(expr 103 + $VAL)    -2
W       realpower.outlet.1000@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D3)     $(expr 106 + $VAL)    -2
A       current.outlet.3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D4)     $(expr 100 + $VAL)    -2
V       voltage.outlet.1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D5)     $(expr 135 + $VAL)    -2
0       frequency.input@ePDU1-LAB       ePDU1-LAB       $(expr $TIME - $D6)     $(expr 170 + $VAL)    -2
%       load.input.L1@ePDU1-LAB 	ePDU1-LAB       $(expr $TIME - $D7)     $(expr 205 + $VAL)    -2
%       load.input.L6@ePDU1-LAB 	ePDU1-LAB       $(expr $TIME - $D8)     $(expr 240 + $VAL)    -2
V       voltage.input.L1-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D9)     $(expr 275 + $VAL)    -2
V       voltage.input.L2-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D10)    $(expr 310 + $VAL)    -2
A       current.input.L1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D11)    $(expr 345 + $VAL)    -2
A       current.input.L3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D12)    $(expr 380 + $VAL)    -2
W       realpower.default@ePDU1-LAB     ePDU1-LAB       $(expr $TIME - $D13)    $(expr 415 + $VAL)    -2
W       realpower.input.L1@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D14)    $(expr 450 + $VAL)    -2
W       realpower.input.L3@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D15)    $(expr 485 + $VAL)    -2
W       power.default@ePDU1-LAB 	ePDU1-LAB       $(expr $TIME - $D16)    $(expr 520 + $VAL)    -2
W       power.input.L1@ePDU1-LAB        ePDU1-LAB       $(expr $TIME - $D17)    $(expr 555 + $VAL)    -2
A       nonsense@ePDU1-LAB      	ePDU1-LAB       $(expr $TIME - $D18)    $(expr 590 + $VAL)    -2
0       status.ups@UPS2-LAB     	UPS2-LAB        $(expr $TIME - $D19)    $(expr 625 + $VAL)    -2
%       load.default@UPS2-LAB   	UPS2-LAB        $(expr $TIME - $D0)     $(expr 660 + $VAL)    -2
V       voltage.output.L1-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D1)     $(expr 695 + $VAL)    -2
W       realpower.output.L1@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D2)     $(expr 730 + $VAL)    -2
A       current.output.L1@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D3)     $(expr 765 + $VAL)    -2
%       charge.battery@UPS2-LAB 	UPS2-LAB        $(expr $TIME - $D4)     $(expr 800 + $VAL)    -2
s       runtime.battery@UPS2-LAB        UPS2-LAB        $(expr $TIME - $D5)     $(expr 835 + $VAL)    -2
W       realpower.default@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D6)     $(expr 870 + $VAL)    -2
V       voltage.output.L2-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D7)     $(expr 905 + $VAL)    -2
W       realpower.output.L2@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D8)     $(expr 940 + $VAL)    -2
A       current.output.L2@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D9)     $(expr 975 + $VAL)    -2
V       voltage.output.L3-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D10)    $(expr 010 + $VAL)    -2
W       realpower.output.L3@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D11)    $(expr 045 + $VAL)    -2
A       current.output.L3@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D12)    $(expr 080 + $VAL)    -2
A       current.output.L8@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D13)    $(expr 115 + $VAL)    -2
W       nonsense@UPS2-LAB       	UPS2-LAB        $(expr $TIME - $D14)    $(expr 150 + $VAL)    -2
W       realpower.outlet.1@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D0)     $(expr 185 + $VAL)    -2
W       realpower.outlet.2@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D1)     $(expr 220 + $VAL)    -2
W       realpower.outlet.15@ePDU2-LAB   ePDU2-LAB       $(expr $TIME - $D2)     $(expr 255 + $VAL)    -2
W       realpower.outlet.1000@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D3)     $(expr 290 + $VAL)    -2
A       current.outlet.3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D4)     $(expr 325 + $VAL)    -2
V       voltage.outlet.1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D5)     $(expr 360 + $VAL)    -2
0       frequency.input@ePDU2-LAB       ePDU2-LAB       $(expr $TIME - $D6)     $(expr 395 + $VAL)    -2
%       load.input.L1@ePDU2-LAB 	ePDU2-LAB       $(expr $TIME - $D7)     $(expr 430 + $VAL)    -2
%       load.input.L6@ePDU2-LAB 	ePDU2-LAB       $(expr $TIME - $D8)     $(expr 465 + $VAL)    -2
V       voltage.input.L1-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D9)     $(expr 500 + $VAL)    -2
V       voltage.input.L2-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D10)    $(expr 535 + $VAL)    -2
A       current.input.L1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D11)    $(expr 570 + $VAL)    -2
A       current.input.L3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D12)    $(expr 605 + $VAL)    -2
W       realpower.default@ePDU2-LAB     ePDU2-LAB       $(expr $TIME - $D13)    $(expr 640 + $VAL)    -2
W       realpower.input.L1@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D14)    $(expr 675 + $VAL)    -2
W       realpower.input.L3@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D15)    $(expr 710 + $VAL)    -2
W       power.default@ePDU2-LAB 	ePDU2-LAB       $(expr $TIME - $D17)    $(expr 745 + $VAL)    -2
W       power.input.L1@ePDU2-LAB        ePDU2-LAB       $(expr $TIME - $D18)    $(expr 780 + $VAL)    -2
A       nonsense@ePDU2-LAB      	ePDU2-LAB       $(expr $TIME - $D19)    $(expr 815 + $VAL)    -2
V	voltage@FEED-LAB		FEED-LAB	$(expr $TIME - $D0 + 1)	$(expr 817 + $VAL)    -2
W	realpower.default@ROOM1-LAB     ROOM1-LAB	$(expr $TIME - $D4 + 4) $(expr 989 + $VAL)    -2
A	current.input.L1@SRV1-LAB	SRV1-LAB	$(expr $TIME - $D7 + 5) $(expr 98 + $VAL)     -2
W	realpower.default@DC-LAB	DC-LAB		$(expr $TIME - $D8 + 6) $(expr 98 + $VAL)     -2
)

    # set the counters
    NPAR=${#PARAMS[*]}
    NSAM=${#SAMPLES[*]}
    SAMPLECNT=$(expr $NSAM / $NPAR - 1)

    # Add all samples including the appropriate topics to DB
    for s in $(seq 0 $SAMPLECNT); do
        SAMPLECURSOR=$(expr $s \* ${NPAR})
        measurement ${SAMPLES[$SAMPLECURSOR]} \
               ${SAMPLES[$(expr $SAMPLECURSOR + 1)]} \
               ${SAMPLES[$(expr $SAMPLECURSOR + 2)]} \
               ${SAMPLES[$(expr $SAMPLECURSOR + 3)]} \
               ${SAMPLES[$(expr $SAMPLECURSOR + 4)]} \
               ${SAMPLES[$(expr $SAMPLECURSOR + 5)]}
    done
}

# START

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

echo "********* current.sh ******************************************************************************"
echo "********* 1. UPS mandatory parameters returned ****************************************************"
echo "***************************************************************************************************"

test_it "UPS_mandatory_parameters_returned"
db_initiate
api_get_json '/metric/current?dev=22' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 2. ePDU mandatory parameters returned ***************************************************"
echo "***************************************************************************************************"

#*#*#*#*#* current.sh - subtest 2 - 5 parameters are missing, 2 parameters redundant

test_it "ePDU_mandatory_parameters_returned"
db_initiate
api_get_json '/metric/current?dev=24' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 3. DC mandatory parameters returned *****************************************************"
echo "***************************************************************************************************"

#*#*#*#*#* current.sh - subtest 3 - 2 parameters are missing

test_it "DC_mandatory_parameters_returned"
db_initiate
api_get_json '/metric/current?dev=19' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 4. UPS older 10 min params test *********************************************************"
echo "***************************************************************************************************"

test_it "UPS_older_10_min_params_test"
db_initiate
STEP=30
DELAY=2000
db_measure $STEP $DELAY 3
api_get_json '/metric/current?dev=24' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 5. UPS under 10 min params test *********************************************************"
echo "***************************************************************************************************"

test_it "UPS_under_10_min_params_test"
db_initiate
STEP=30
DELAY=599
db_measure $STEP $DELAY 1
api_get_json '/metric/current?dev=24' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 6. The last measurement is used *********************************************************"
echo "***************************************************************************************************"

test_it "The_last_measurement_is_used"
db_initiate
for i in $(seq 0 2);do
    DELAY=$(expr 600 - $i \* 180)
    STEP=7
    VAL=$(expr $i \* 2000)
    db_measure $STEP $DELAY $VAL
done
api_get_json '/metric/current?dev=24' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 7. All kinds of devices *****************************************************************"
echo "***************************************************************************************************"

test_it "All_kinds_of_devices"
db_initiate
for i in $(seq 0 2);do
    DELAY=$(expr 600 - $i \* 180)
    STEP=7
    VAL=$(expr $i \* 2000)
    db_measure $STEP $DELAY $VAL
done
for j in $(seq 19 35);do
    api_get_json "/metric/current?dev=$j" >&5
done
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 8. Requested ID does not exist **********************************************************"
echo "***************************************************************************************************"

test_it "Requested_ID_does_not_exist"
db_initiate
api_get_json '/metric/current?dev=333' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 9. Missing ID ***************************************************************************"
echo "***************************************************************************************************"

test_it "Missing_ID"
db_initiate
curlfail_push_expect_400
api_get_json '/metric/current?dev=' >&5
print_result $?
curlfail_pop

echo "********* current.sh ******************************************************************************"
echo "********* 10. Several IDs some missing ************************************************************"
echo "***************************************************************************************************"

test_it "Several_IDs_some_missing"
db_initiate
api_get_json '/metric/current?dev=abc,21,28,555,35,45' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 11. The format of value list error 1 ****************************************************"
echo "***************************************************************************************************"

test_it "The_format_of_value_list_error_1"
db_initiate
api_get_json '/metric/current?dev=abc,21,28%20%20555,35,45' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 12. The format of value list error 2 ****************************************************"
echo "***************************************************************************************************"

test_it "The_format_of_value_list_error_2"
db_initiate
api_get_json '/metric/current?dev=abc,21,28;555,35,45' >&5
print_result $?

echo
echo "###################################################################################################"
echo "********* current.sh ******************************** END *****************************************"
echo "###################################################################################################"
echo

