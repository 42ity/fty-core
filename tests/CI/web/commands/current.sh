#!/bin/sh

# Add the library
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }

# Add the first line of the sql file and create it
echo "use box_utf8;" > /tmp/tmp.sql

# Function definition - Add the measurement chosen from the SAMPLES variable
measurement() {
UNIT=$1
TOPIC=$2
DEVICE=$3
TIMESTAMP=$4
VALUE=$5
SCALE=$6

SLCT="select id from t_bios_measurement_topic where topic='$TOPIC'"
TOP_ID="$(echo $SLCT | mysql -u root box_utf8 -N)"
if [ "$TOP_ID" == "" ]; then
   sqlline="INSERT INTO t_bios_measurement_topic (device_id, units,topic) SELECT r.id_discovered_device,'$UNIT','$TOPIC' FROM t_bios_asset_element AS e,t_bios_monitor_asset_relation AS r WHERE e.name = '$DEVICE' AND e.id_asset_element = r.id_asset_element;"
   echo $sqlline >> /tmp/tmp.sql
   sqlline="set @topic_id = LAST_INSERT_ID();"
else
   sqlline="set @topic_id = $TOP_ID;"
fi
#echo "sqlline = $sqlline"
echo $sqlline >> /tmp/tmp.sql
# measurement
sqlline="INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ($TIMESTAMP, $VALUE, $SCALE, @topic_id);"
echo $sqlline >> /tmp/tmp.sql
loaddb_file "/tmp/tmp.sql"
echo "use box_utf8;" > /tmp/tmp.sql
}

db_initiate(){
DB_LOADDIR="$CHECKOUTDIR/tools"
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql"
DB_ASSET_DEFAULT="initdb_ci_patch_2.sql"

for data in "$DB_BASE" "$DB_DATA" "$DB_DATA_TESTREST"; do
    loaddb_file "$DB_LOADDIR/$data" || return $?
done
}

db_measure(){
# set actual time
TIME=$(date +%s)
STEP=$1
VAL=$3
D0=$2
D1=$(expr $D0 - $STEP)
D2=$(expr $D1 - $STEP)
D3=$(expr $D2 - $STEP)
D4=$(expr $D3 - $STEP)
D5=$(expr $D4 - $STEP)
echo "D5 = $D5"
echo "TIME5 = $(expr $TIME - $D1)"
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
NULL    status.ups@UPS1-LAB     UPS1-LAB    $(expr $TIME - $D0)     $(expr 400 + $VAL)     -2
%       load.default@UPS1-LAB   UPS1-LAB        $(expr $TIME - $D0)     $(expr 435 + $VAL)     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D1)     $(expr 470 + $VAL)     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D2)     $(expr 475 + $VAL)     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D3)     $(expr 478 + $VAL)     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D4)     $(expr 473 + $VAL)     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D5)     $(expr 471 + $VAL)     -2
W       realpower.output.L1@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D6)     $(expr 505 + $VAL)     -2
A       current.output.L1@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D7)     $(expr 540 + $VAL)     -2
%       charge.battery@UPS1-LAB UPS1-LAB        $(expr $TIME - $D8)             $(expr 575 + $VAL)     -2
s       runtime.battery@UPS1-LAB        UPS1-LAB        $(expr $TIME - $D9)     $(expr 610 + $VAL)     -2
W       realpower.default@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D10)    $(expr 645 + $VAL)     -2
V       voltage.output.L2-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D11)    $(expr 680 + $VAL)     -2
W       realpower.output.L2@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D12)    $(expr 715 + $VAL)     -2
A       current.output.L2@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D13)    $(expr 750 + $VAL)     -2
V       voltage.output.L3-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D14)    $(expr 785 + $VAL)     -2
W       realpower.output.L3@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D15)    $(expr 820 + $VAL)     -2
A       current.output.L3@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D16)    $(expr 855 + $VAL)     -2
A       current.output.L8@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D17)    $(expr 890 + $VAL)     -2
W       nonsense@UPS1-LAB       UPS1-LAB        $(expr $TIME - $D18)    $(expr 925 + $VAL)     -2
W       realpower.outlet.1@ePDU1-LAB    ePDU1-LAB        $(expr $TIME - $D0)     $(expr 960 + $VAL)     -2
W       realpower.outlet.2@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D1)     $(expr 995 + $VAL)     -2
W       realpower.outlet.15@ePDU1-LAB   ePDU1-LAB       $(expr $TIME - $D2)     $(expr 1030 + $VAL)    -2
W       realpower.outlet.$(expr 1000 + $VAL)@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D3)     $(expr 1065 + $VAL)    -2
A       current.outlet.3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D4)     $(expr 1100 + $VAL)    -2
V       voltage.outlet.1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D5)     $(expr 1135 + $VAL)    -2
0       frequency.input@ePDU1-LAB       ePDU1-LAB       $(expr $TIME - $D6)     $(expr 1170 + $VAL)    -2
%       load.input.L1@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D7)     $(expr 1205 + $VAL)    -2
%       load.input.L6@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D8)     $(expr 1240 + $VAL)    -2
V       voltage.input.L1-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D9)      $(expr 1275 + $VAL)    -2
V       voltage.input.L2-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D10)     $(expr 1310 + $VAL)    -2
A       current.input.L1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D11)     $(expr 1345 + $VAL)    -2
A       current.input.L3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D12)     $(expr 1380 + $VAL)    -2
W       realpower.default@ePDU1-LAB     ePDU1-LAB       $(expr $TIME - $D13)     $(expr 1415 + $VAL)    -2
W       realpower.input.L1@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D14)     $(expr 1450 + $VAL)    -2
W       realpower.input.L3@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D15)     $(expr 1485 + $VAL)    -2
W       power.default@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D16)     $(expr 1520 + $VAL)    -2
W       power.input.L1@ePDU1-LAB        ePDU1-LAB       $(expr $TIME - $D17)      $(expr 1555 + $VAL)    -2
A       nonsense@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D18)       $(expr 1590 + $VAL)    -2
0       status.ups@UPS2-LAB     UPS2-LAB       $(expr $TIME - $D19)     $(expr 1625 + $VAL)    -2
%       load.default@UPS2-LAB   UPS2-LAB        $(expr $TIME - $D0)     $(expr 1660 + $VAL)    -2
V       voltage.output.L1-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D1)     $(expr 1695 + $VAL)    -2
W       realpower.output.L1@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D2)     $(expr 1730 + $VAL)    -2
A       current.output.L1@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D3)     $(expr 1765 + $VAL)    -2
%       charge.battery@UPS2-LAB UPS2-LAB        $(expr $TIME - $D4)     $(expr 1800 + $VAL)    -2
s       runtime.battery@UPS2-LAB        UPS2-LAB        $(expr $TIME - $D5)     $(expr 1835 + $VAL)    -2
W       realpower.default@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D6)     $(expr 1870 + $VAL)    -2
V       voltage.output.L2-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D7)     $(expr 1905 + $VAL)    -2
W       realpower.output.L2@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D8)     $(expr 1940 + $VAL)    -2
A       current.output.L2@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D9)      $(expr 1975 + $VAL)    -2
V       voltage.output.L3-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D10)     $(expr 2010 + $VAL)    -2
W       realpower.output.L3@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D11)     $(expr 2045 + $VAL)    -2
A       current.output.L3@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D12)     $(expr 2080 + $VAL)    -2
A       current.output.L8@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D13)     $(expr 2115 + $VAL)    -2
W       nonsense@UPS2-LAB       UPS2-LAB        $(expr $TIME - $D14)     $(expr 2150 + $VAL)    -2
W       realpower.outlet.1@ePDU2-LAB    ePDU2-LAB        $(expr $TIME - $D0)     $(expr 2185 + $VAL)    -2
W       realpower.outlet.2@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D1)     $(expr 2220 + $VAL)    -2
W       realpower.outlet.15@ePDU2-LAB   ePDU2-LAB       $(expr $TIME - $D2)     $(expr 2255 + $VAL)    -2
W       realpower.outlet.1000@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D3)     $(expr 2290 + $VAL)    -2
A       current.outlet.3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D4)     $(expr 2325 + $VAL)    -2
V       voltage.outlet.1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D5)     $(expr 2360 + $VAL)    -2
0       frequency.input@ePDU2-LAB       ePDU2-LAB       $(expr $TIME - $D6)     $(expr 2395 + $VAL)    -2
%       load.input.L1@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D7)     $(expr 2430 + $VAL)    -2
%       load.input.L6@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D8)     $(expr 2465 + $VAL)    -2
V       voltage.input.L1-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D9)      $(expr 2500 + $VAL)    -2
V       voltage.input.L2-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D10)     $(expr 2535 + $VAL)    -2
A       current.input.L1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D11)     $(expr 2570 + $VAL)    -2
A       current.input.L3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D12)     $(expr 2605 + $VAL)    -2
W       realpower.default@ePDU2-LAB     ePDU2-LAB       $(expr $TIME - $D13)     $(expr 2640 + $VAL)    -2
W       realpower.input.L1@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D14)     $(expr 2675 + $VAL)    -2
W       realpower.input.L3@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D15)     $(expr 2710 + $VAL)    -2
W       power.default@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D17)     $(expr 2745 + $VAL)    -2
W       power.input.L1@ePDU2-LAB        ePDU2-LAB       $(expr $TIME - $D18)      $(expr 2780 + $VAL)    -2
A       nonsense@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D19)       $(expr 2815 + $VAL)    -2
)

echo ${SAMPLES[*]}

#Define the measurements samples
SAMPLES1=(
NULL    status.ups@UPS1-LAB     UPS1-LAB    $(expr $TIME - $D0)     400     -2
%       load.default@UPS1-LAB   UPS1-LAB        $(expr $TIME - $D0)     435     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D1)     470     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D2)     475     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D3)     478     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D4)     473     -2
V       voltage.output.L1-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D5)     471     -2
W       realpower.output.L1@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D6)     505     -2
A       current.output.L1@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D7)     540     -2
%       charge.battery@UPS1-LAB UPS1-LAB        $(expr $TIME - $D8)     575     -2
s       runtime.battery@UPS1-LAB        UPS1-LAB        $(expr $TIME - $D9)     610     -2
W       realpower.default@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D10)    645     -2
V       voltage.output.L2-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D11)    680     -2
W       realpower.output.L2@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D12)    715     -2
A       current.output.L2@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D13)    750     -2
V       voltage.output.L3-N@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D14)    785     -2
W       realpower.output.L3@UPS1-LAB    UPS1-LAB        $(expr $TIME - $D15)    820     -2
A       current.output.L3@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D16)    855     -2
A       current.output.L8@UPS1-LAB      UPS1-LAB        $(expr $TIME - $D17)    890     -2
W       nonsense@UPS1-LAB       UPS1-LAB        $(expr $TIME - $D18)    925     -2
W       realpower.outlet.1@ePDU1-LAB    ePDU1-LAB        $(expr $TIME - $D0)     910     -2
W       realpower.outlet.2@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D1)     995     -2
W       realpower.outlet.15@ePDU1-LAB   ePDU1-LAB       $(expr $TIME - $D2)     1030    -2
W       realpower.outlet.1000@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D3)     1065    -2
A       current.outlet.3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D4)     1100    -2
V       voltage.outlet.1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D5)     1135    -2
0       frequency.input@ePDU1-LAB       ePDU1-LAB       $(expr $TIME - $D6)     1170    -2
%       load.input.L1@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D7)     1205    -2
%       load.input.L6@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D8)     1240    -2
V       voltage.input.L1-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D9)      1275    -2
V       voltage.input.L2-N@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D10)     1310    -2
A       current.input.L1@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D11)     1345    -2
A       current.input.L3@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D12)     1380    -2
W       realpower.default@ePDU1-LAB     ePDU1-LAB       $(expr $TIME - $D13)     1415    -2
W       realpower.input.L1@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D14)     1450    -2
W       realpower.input.L3@ePDU1-LAB    ePDU1-LAB       $(expr $TIME - $D15)     1485    -2
W       power.default@ePDU1-LAB ePDU1-LAB       $(expr $TIME - $D16)     1520    -2
W       power.input.L1@ePDU1-LAB        ePDU1-LAB       $(expr $TIME - $D17)      1555    -2
A       nonsense@ePDU1-LAB      ePDU1-LAB       $(expr $TIME - $D18)       1590    -2
0       status.ups@UPS2-LAB     UPS2-LAB       $(expr $TIME - $D19)     1625    -2
%       load.default@UPS2-LAB   UPS2-LAB        $(expr $TIME - $D0)     1660    -2
V       voltage.output.L1-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D1)     1695    -2
W       realpower.output.L1@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D2)     1730    -2
A       current.output.L1@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D3)     1765    -2
%       charge.battery@UPS2-LAB UPS2-LAB        $(expr $TIME - $D4)     1800    -2
s       runtime.battery@UPS2-LAB        UPS2-LAB        $(expr $TIME - $D5)     1835    -2
W       realpower.default@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D6)     1870    -2
V       voltage.output.L2-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D7)     1905    -2
W       realpower.output.L2@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D8)     1940    -2
A       current.output.L2@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D9)      1975    -2
V       voltage.output.L3-N@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D10)     2010    -2
W       realpower.output.L3@UPS2-LAB    UPS2-LAB        $(expr $TIME - $D11)     2045    -2
A       current.output.L3@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D12)     2080    -2
A       current.output.L8@UPS2-LAB      UPS2-LAB        $(expr $TIME - $D13)     2115    -2
W       nonsense@UPS2-LAB       UPS2-LAB        $(expr $TIME - $D14)     2150    -2
W       realpower.outlet.1@ePDU2-LAB    ePDU2-LAB        $(expr $TIME - $D0)     2185    -2
W       realpower.outlet.2@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D1)     2220    -2
W       realpower.outlet.15@ePDU2-LAB   ePDU2-LAB       $(expr $TIME - $D2)     2255    -2
W       realpower.outlet.1000@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D3)     2290    -2
A       current.outlet.3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D4)     2325    -2
V       voltage.outlet.1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D5)     2360    -2
0       frequency.input@ePDU2-LAB       ePDU2-LAB       $(expr $TIME - $D6)     2395    -2
%       load.input.L1@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D7)     2430    -2
%       load.input.L6@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D8)     2465    -2
V       voltage.input.L1-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D9)      2500    -2
V       voltage.input.L2-N@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D10)     2535    -2
A       current.input.L1@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D11)     2570    -2
A       current.input.L3@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D12)     2605    -2
W       realpower.default@ePDU2-LAB     ePDU2-LAB       $(expr $TIME - $D13)     2640    -2
W       realpower.input.L1@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D14)     2675    -2
W       realpower.input.L3@ePDU2-LAB    ePDU2-LAB       $(expr $TIME - $D15)     2710    -2
W       power.default@ePDU2-LAB ePDU2-LAB       $(expr $TIME - $D17)     2745    -2
W       power.input.L1@ePDU2-LAB        ePDU2-LAB       $(expr $TIME - $D18)      2780    -2
A       nonsense@ePDU2-LAB      ePDU2-LAB       $(expr $TIME - $D19)       2815    -2
)


# set the counters
NPAR=${#PARAMS[*]}
NSAM=${#SAMPLES[*]}
SAMPLECNT=$(expr $NSAM / $NPAR - 1)

# Add all samples including the appropriate topics to DB
for s in $(seq 0 $SAMPLECNT);do
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
# set the counters
NPAR=${#PARAMS[*]}
NSAM=${#SAMPLES[*]}
SAMPLECNT=$(expr $NSAM / $NPAR - 1)

echo "###################################################################################################"
echo "********* current.sh ******************************* START ****************************************"
echo "###################################################################################################"
echo

echo "********* current.sh ******************************************************************************"
echo "********* 1. UPS mandatory parameters returned ****************************************************"
echo "***************************************************************************************************"

test_it "UPS_mandatory_parameters_returned"
api_get_json '/metric/current?dev=22' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 2. ePDU mandatory parameters returned ***************************************************"
echo "***************************************************************************************************"

#*#*#*#*#* current.sh - subtest 2 - 5 parameters are missing, 2 parameters redundant

test_it "ePDU_mandatory_parameters_returned"
api_get_json '/metric/current?dev=24' >&5
print_result $?

echo "********* current.sh ******************************************************************************"
echo "********* 3. DC mandatory parameters returned *****************************************************"
echo "***************************************************************************************************"

#*#*#*#*#* current.sh - subtest 3 - 2 parameters are missing

test_it "DC_mandatory_parameters_returned"
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
echo "********* 6. UPS under 10 min params test *********************************************************"
echo "***************************************************************************************************"

test_it "UPS_under_10_min_params_test"
db_initiate

for i in $(seq 0 2);do
   DELAY=$(expr 1800 - $i \* 11)
   STEP=100
   VAL=$(expr $i \* 20)
#   STEP=$(expr 30 - $i \* 2)
   db_measure $STEP $DELAY $VAL
done
sleep 120
api_get_json '/metric/current?dev=24' >&5
print_result $?



