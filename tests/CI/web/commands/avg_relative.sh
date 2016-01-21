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
#! \file   avg_relative.sh
#  \brief  CI tests for avg "relative" parameter
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

echo
echo "###################################################################################################"
echo "********* avg_relative.sh ************************** START ****************************************"
echo "###################################################################################################"
echo
[ x"${JSONSH_CLI_DEFINED-}" = xyes ] || CODE=127 die "jsonsh_cli() not defined"

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
    LOADDB_FILE_REMOTE_SLEEP=3 loaddb_current || return $?
    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
    sqlline="INSERT INTO t_bios_discovered_device (id_discovered_device,name,id_device_type) VALUES (NULL, 'DC-LAB', 1);"
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file "$DB_TMPSQL_FILE_CURRENT"
    echo "use ${DATABASE};" > "${DB_TMPSQL_FILE_CURRENT}"
    SEL_ID="select id_discovered_device from t_bios_discovered_device where name='DC-LAB'"
    TOP_ID="$(do_select "$SEL_ID")"
    sqlline="INSERT INTO t_bios_monitor_asset_relation (id_ma_relation,id_discovered_device,id_asset_element) VALUES (NULL, $TOP_ID, 19);"
    echo "$sqlline" >> "${DB_TMPSQL_FILE_CURRENT}"
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file "$DB_TMPSQL_FILE_CURRENT"
}

db_measure(){
    # set actual time
    TIME=$(date +%s)

    # set the header names of the params
    PARAMS=(units topic name timestamp value scale)

    #Define the measurements samples
    SAMPLES=(
W       realpower.default@UPS1-LAB      UPS1-LAB        $(expr $TIME - 3000000)     2000   -2
%       load.default@UPS1-LAB   	UPS1-LAB        $(expr $TIME - 1000000)     50     -2
W       realpower.output.L1@UPS1-LAB    UPS1-LAB        $(expr $TIME - 100000)      1000   -2
A       current.output.L1@UPS1-LAB      UPS1-LAB        $(expr $TIME - 50000)       200    -2
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

api_get_json_ntstp(){
    [ -z ${RES+x} ] && RES=0
    GTJSON=`api_get_json $1`
    RES="$(expr $RES + $?)"
    echo $GTJSON | sed 's/[[:digit:]]\{10\}/1111111/g' >&5
}


# START

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script
db_initiate
db_measure

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 1. AVG for measurement with < 24 hod old timestamp **************************************"
echo "***************************************************************************************************"
test_it "AVG_for_measurement_with_<_24_hod_old_timestamp"
curlfail_push_expect_noerrors
RES=0
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 2. AVG for measurement with > 24 hod old timestamp **************************************"
echo "***************************************************************************************************"
test_it "AVG_for_measurement_with_>_24_hod_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.output.L1'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 3. AVG for measurement with > 7 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "AVG_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=load.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=load.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=load.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=load.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 4. AVG for measurement with > 30 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "AVG_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 5. AVG for measurement with < 24 hod old timestamp without start-end-ts *****************"
echo "***************************************************************************************************"
test_it "AVG_for_measurement_with_<_24_hod_old_timestamp_without_start-end-ts"
api_get_json_ntstp '/metric/computed/average?type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?type=arithmetic_mean&step=15m&element_id=22&source=current.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 6. MIN for measurement with > 24 hod old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MIN_for_measurement_with_>_24_hod_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.output.L1'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 7. MIN for measurement with > 7 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MIN_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=load.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=load.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=load.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=load.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 8. MIN for measurement with > 30 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MIN_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=min&step=15m&element_id=22&source=realpower.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 9. MIN for measurement with < 24 hod old timestamp without start-end-ts *****************"
echo "***************************************************************************************************"
test_it "MIN_for_measurement_with_<_24_hod_old_timestamp_without_start-end-ts"
api_get_json_ntstp '/metric/computed/average?type=min&step=15m&element_id=22&source=current.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?type=min&step=15m&element_id=22&source=current.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?type=min&step=15m&element_id=22&source=current.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 10. MAX for measurement with > 24 hod old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MAX_for_measurement_with_>_24_hod_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.output.L1'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 11. MAX for measurement with > 7 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MAX_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=load.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=load.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=load.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=load.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 12. MAX for measurement with > 30 days old timestamp **************************************"
echo "***************************************************************************************************"
test_it "MAX_for_measurement_with_>_7_days_old_timestamp"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.default'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.default&relative=24h'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.default&relative=7d'
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.default&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 13. MAX for measurement with < 24 hod old timestamp without start-end-ts *****************"
echo "***************************************************************************************************"
test_it "MAX_for_measurement_with_<_24_hod_old_timestamp_without_start-end-ts"
api_get_json_ntstp '/metric/computed/average?type=max&step=15m&element_id=22&source=current.output.L1&relative=24h'
api_get_json_ntstp '/metric/computed/average?type=max&step=15m&element_id=22&source=current.output.L1&relative=7d'
api_get_json_ntstp '/metric/computed/average?type=max&step=15m&element_id=22&source=current.output.L1&relative=30d'
print_result $RES;RES=0

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 14. MAX for measurement with empty relative parameter ***********************************"
echo "***************************************************************************************************"
test_it "MAX_for_measurement_with_empty_relative_parameter"
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=max&step=15m&element_id=22&source=realpower.default&relative='
print_result $RES;RES=0
curlfail_pop

echo
echo "********* avg_relative.sh *************************************************************************"
echo "********* 15. Wrong value of relative **************************************************************"
echo "***************************************************************************************************"

test_it "Wrong_value_of_relative"
curlfail_push_expect_400
api_get_json_ntstp '/metric/computed/average?end_ts=20160202000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=15m&element_id=22&source=realpower.default&relative=40d'
print_result $RES;RES=0
curlfail_pop
