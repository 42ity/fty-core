#
# Copyright (c) 2016 Eaton
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
#! \file   trends.sh
#  \brief  CI tests for avg, min, max, trends - datacenter indicators
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

echo
echo "###################################################################################################"
echo "********* trends.sh ******************************** START ****************************************"
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
    LOADDB_FILE_REMOTE_SLEEP=0 loaddb_file "${DB_TMPSQL_FILE_CURRENT}"
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
    TIME=$(date -u '+%s')

    # set the header names of the params
    PARAMS=(units topic name timestamp value scale)
    START_DAY=$(date -u -d '00:00 today' '+%s')
    #Define the measurements samples
    SAMPLES=(
W   realpower.default_min_30d@DC-LAB                DC-LAB    $START_DAY     100     -2
W   realpower.default_min_7d@DC-LAB                 DC-LAB    $START_DAY     200     -2
W   realpower.default_min_24h@DC-LAB                DC-LAB    $START_DAY     400     -2
W   realpower.default_max_30d@DC-LAB                DC-LAB    $START_DAY     5000    -2
W   realpower.default_max_7d@DC-LAB                 DC-LAB    $START_DAY     4000    -2
W   realpower.default_max_24h@DC-LAB                DC-LAB    $START_DAY     3000    -2
W   realpower.default_arithmetic_mean_30d@DC-LAB    DC-LAB    $START_DAY     2000    -2
W   realpower.default_arithmetic_mean_7d@DC-LAB     DC-LAB    $START_DAY     1000    -2
W   realpower.default_arithmetic_mean_24h@DC-LAB    DC-LAB    $START_DAY     4000    -2
C   average.temperature_arithmetic_mean_24h@DC-LAB  DC-LAB    $START_DAY     2000    -2
C   average.temperature_arithmetic_mean_7d@DC-LAB   DC-LAB    $START_DAY     2500    -2
C   average.temperature_arithmetic_mean_30d@DC-LAB  DC-LAB    $START_DAY     1800    -2
%   average.humidity_arithmetic_mean_24h@DC-LAB     DC-LAB    $START_DAY     5000    -2
%   average.humidity_arithmetic_mean_7d@DC-LAB      DC-LAB    $START_DAY     8000    -2
%   average.humidity_arithmetic_mean_30d@DC-LAB     DC-LAB    $START_DAY     6000    -2
W   realpower.default@DC-LAB                        DC-LAB    $(expr $TIME - 10)          3000    -2
C   average.temperature@DC-LAB                      DC-LAB    $(expr $TIME - 10)          4000    -2
%   average.humidity@DC-LAB                         DC-LAB    $(expr $TIME - 10)          7000    -2
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
    api_get_json $1 >&5
    RES="$(expr $RES + $?)"
}


# START

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script
db_initiate
db_measure

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 1. Indicators - avg_power_last_day_week_month *******************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_power_last_day_week_month"
curlfail_push_expect_noerrors
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_power_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_power_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_power_last_month'
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 2. Indicators - avg_temperature_last_day_week_month *************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_temperature_last_day_week_month"
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_temperature_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_temperature_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_temperature_last_month'
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 3. Indicators - avg_humidity_last_day_week_month *************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_humidity_last_day_week_month"
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_humidity_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_humidity_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=avg_humidity_last_month'
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 4. Indicators - avg_power_last_day_week_month *******************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_power_last_day_week_month"
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_power_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_power_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_power_last_month'
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 5. Indicators - avg_temperature_last_day_week_month *************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_temperature_last_day_week_month"
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_temperature_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_temperature_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_temperature_last_month'
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 6. Indicators - avg_humidity_last_day_week_month *************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_avg_humidity_last_day_week_month"
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_humidity_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_humidity_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_humidity_last_month'
print_result $RES;RES=0
curlfail_pop

echo "********* trends.sh *******************************************************************************"
echo "********* 7. Indicators - min/max last_day_week_month *************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_min/max_last_day_week_month"
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=min_power_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=min_power_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=min_power_last_month'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=max_power_last_day'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=max_power_last_week'
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=max_power_last_month'
RES=0
print_result $RES;RES=0

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 8. Indicators - wrong arg1 - not DC *****************************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_wrong arg1_-_not DC"
curlfail_push_expect_404
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=2&arg2=trend_power_last_day'
print_result $RES;RES=0
curlfail_pop

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 9. Indicators - wrong format ************************************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_wrong format"
curlfail_push_expect_400
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg3=trend_power_last_day'
print_result $RES;RES=0
curlfail_pop

echo
echo "********* trends.sh *******************************************************************************"
echo "********* 10. Indicators - wrong parameter value **************************************************"
echo "***************************************************************************************************"
test_it "Indicators_-_wrong parameter_value"
curlfail_push_expect_400
RES=0
api_get_json_ntstp '/metric/computed/datacenter_indicators?arg1=19&arg2=trend_pwr_last_day'
print_result $RES;RES=0
curlfail_pop

