#!/bin/bash

# Copyright (C) 2014-2015 Eaton
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
# Author(s): Radomir Vrajik <RadomirVrajik@Eaton.com>,
#
# Description: tests the csv import

# ***** ABBREVIATIONS *****
    # *** SUT - System Under Test - remote server with BIOS
    # *** MS - Management Station - local server with this script
    # *** TAB - Tabelator

# ***** DESCRIPTION *****
    # *** test imports the assets from csv file with tab used like separator
    # *** tests creates initial (near to empty assets) using database/mysql/initdb.sql file.
    # *** tests compares the exported tableis files with expected (only INSERT command)

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT should be passed as parameter -sp <value>
    # *** it is currently from interval <2206;2209>
    # *** must run as root without using password
    # *** BIOS image must be installed and running on SUT
    # *** directories containing database/mysql/initdb.sql tools/bam_vte_tab_import.csv present on MS
    # *** tests/CI directory (on MS) contains weblib.sh and scriptlib.sh library files

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)
[ -z "${SUT_WEB_SCHEMA-}" ] && SUT_WEB_SCHEMA="http"

    # *** read parameters if present
while [ $# -gt 0 ]; do
    case "$1" in
        --port-ssh|--sut-port-ssh|-sp)
            SUT_SSH_PORT="$2"
            shift 2
            ;;
        --port-web|--sut-port-web|-wp|-o|--port)
            SUT_WEB_PORT="$2"
            shift 2
            ;;
        --host|--machine|-s|-sh|--sut|--sut-host)
            SUT_HOST="$2"
            shift 2
            ;;
        --use-https|--sut-web-https)    SUT_WEB_SCHEMA="https"; shift;;
        --use-http|--sut-web-http)      SUT_WEB_SCHEMA="http"; shift;;
        --sut-user|-su)
            SUT_USER="$2"
            shift 2
            ;;
        -u|--user|--bios-user)
            BIOS_USER="$2"
            shift 2
            ;;
        -p|--passwd|--bios-passwd)
            BIOS_PASSWD="$2"
            shift 2
            ;;
        -s|--service)
            SASL_SERVICE="$2"
            shift 2
            ;;
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            break
            ;;
    esac
done

# default values:
[ -z "$SUT_USER" ] && SUT_USER="root"
[ -z "$SUT_HOST" ] && SUT_HOST="debian.roz53.lab.etn.com"
# port used for ssh requests:
[ -z "$SUT_SSH_PORT" ] && SUT_SSH_PORT="2206"
# port used for REST API requests:
if [ -z "$SUT_WEB_PORT" ]; then
    if [ -n "$BIOS_PORT" ]; then
        SUT_WEB_PORT="$BIOS_PORT"
    else
        SUT_WEB_PORT=$(expr $SUT_SSH_PORT + 8000)
        [ "$SUT_SSH_PORT" -ge 2200 ] && \
            SUT_WEB_PORT=$(expr $SUT_WEB_PORT - 2200)
    fi
fi
# unconditionally calculated values
BASE_URL="${SUT_WEB_SCHEMA}://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || die "Can not include web script library"

logmsg_info "Will use BASE_URL = '$BASE_URL'"

determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

# ***** INIT DB *****
    # *** write power rack base test data to DB on SUT
set -o pipefail 2>/dev/null || true
set -e
loaddb_file "$DB_BASE" 2>&1 | tee "$CHECKOUTDIR/vte-tab-${_SCRIPT_NAME}.log"

# NOTE: This test verifies that with our standard configuration of the VTE
# and its database we can import our assets, so we do not apply hacks like
# the DB_ASSET_TAG_NOT_UNIQUE="initdb_ci_patch.sql" used elsewhere.
# If this test fails, then sources and/or sample SQL/CSV data must be fixed.

set +e

# Try to accept the BIOS license on server
( . $CHECKOUTDIR/tests/CI/web/commands/00_license-CI-forceaccept.sh.test 5>&2 ) || \
    logmsg_warn "BIOS license not accepted on the server, subsequent tests may fail"

# ***** POST THE CSV FILE *****
ASSET="$CSV_LOADDIR_BAM/bam_vte_tab_import.csv"

# Import the bam_vte_tab_import.csv file
api_auth_post_file_form /asset/import assets="@$ASSET" | tee "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

NUM_EXPECTED=13
grep -q '"imported_lines" : '"$NUM_EXPECTED" "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log" || \
    die "ERROR : 'Test of the number of imported lines			FAILED  (not $NUM_EXPECTED)'"
echo "Test of the number of imported lines			PASSED"

for NUM in 9 10 11 16 18 20 21 22 23 24 25 27 28 29 ; do
    grep -q "\[ $NUM," "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log" || \
        die "ERROR : 'Test of the line   $NUM 				FAILED'"
    echo "Test of the line  $NUM  					PASSED"
done
#ELEMENT="\(1,'DC-LAB1',2,10,NULL,'active',1,1,'EATON12310'\),\(2,'ROOM-01',3,10,1,'active',2,0,'EATON12311'\),\(3,'ANNEX-01',3,10,1,'active',3,1,'EATON12312'\),\(4,'CAGE-01',1,10,2,'active',4,1,'EATON12313'\),\(5,'ROW-01',4,10,2,'active',5,1,'EATON12314'\),\(6,'ROW-02',4,10,2,'active',5,1,'EATON12315'\),\(7,'RACK-01',5,10,5,'active',5,1,'EATON12345'\),\(9,'CUSTOMER_02',1,10,1,'nonactive',2,1,'EATON12318'\),\(10,'MAIN_LAB',6,6,1,'nonactive',3,1,'EATON12319'\),\(11,'GENSET_01',6,2,3,'spare',5,1,'EATON12347'\),\(12,'GENSET_03',6,4,3,'nonactive',5,1,'EATON12349'\),\(13,'ATS_01',6,7,2,'active',5,1,'EATON12341'\),\(16,'SRV2_LAB',6,5,7,'active',5,1,'EATON12344'\)"

#ELEMENT="\(1,'DC-LAB1',2,10,NULL,'active',1,1,'EATON12310'\),\(2,'ROOM-01',3,10,1,'active',2,0,'EATON12311'\),\(3,'ANNEX-01',3,10,1,'active',3,1,'EATON12312'\),\(4,'CAGE-01',1,10,2,'active',4,1,'EATON12313'\),\(5,'ROW-01',4,10,2,'active',5,1,'EATON12314'\),\(6,'ROW-02',4,10,2,'active',5,1,'EATON12315'\),\(7,'RACK-01',5,10,5,'active',5,1,'EATON12345'\),\(8,'GROUP1-LAB',1,10,1,'nonactive',1,1,'EATON12316'\),\(9,'CUSTOMER_02',1,10,1,'nonactive',2,1,'EATON12318'\),\(10,'MAIN_LAB',6,6,1,'nonactive',3,1,'EATON12319'\),\(11,'MAIN3P_LAB',6,6,1,'nonactive',4,0,'EATON12320'\),\(12,'GENSET_01',6,2,3,'spare',5,1,'EATON12347'\),\(13,'GENSET_03',6,4,3,'nonactive',5,1,'EATON12349'\),\(14,'ATS_01',6,7,2,'active',5,1,'EATON12341'\),\(16,'ePDU3-LAB',6,3,7,'active',5,1,' 3xC19'\),\(17,'SRV2_LAB',6,5,7,'active',5,1,'EATON12344'\)"

ELEMENT="\(1,'DC-LAB1',2,10,NULL,'active',1,1,'EATON12310'\),\(2,'ROOM-01',3,10,1,'active',2,0,'EATON12311'\),\(3,'ANNEX-01',3,10,1,'active',3,1,'EATON12312'\),\(4,'CAGE-01',1,10,2,'active',4,1,'EATON12313'\),\(5,'ROW-01',4,10,2,'active',5,1,'EATON12314'\),\(6,'ROW-02',4,10,2,'active',5,1,'EATON12315'\),\(7,'RACK-01',5,10,5,'active',5,1,'EATON12345'\),\(9,'CUSTOMER_02',1,10,1,'active',2,1,'EATON12318'\),\(10,'MAIN_LAB',6,6,1,'nonactive',3,1,'EATON12319'\),\(11,'GENSET_01',6,2,3,'spare',5,1,'EATON12347'\),\(12,'GENSET_03',6,4,3,'spare',5,1,'EATON12349'\),\(13,'ATS_01',6,7,2,'active',5,1,'EATON12341'\),\(16,'SRV2_LAB',6,5,7,'active',5,1,'EATON12344'\)"

I=$(do_dumpdb t_bios_asset_element | awk "/INSERT/ && /ROOM-01/" | egrep -c "$ELEMENT" ) 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_element FAILED'"
echo 'Test of the table t_bios_asset_element 			PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

ELEMENT="\(2,'datacenter'\),\(6,'device'\),\(1,'group'\),\(5,'rack'\),\(3,'room'\),\(4,'row'\)"
I=$(do_dumpdb t_bios_asset_element_type | grep INSERT | egrep -c "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_element_type FAILED'"
echo 'Test of the table t_bios_asset_element_type 		PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

ELEMENT="\(1,'company','EATON',1,0\),\(2,'description','Lab DC',1,0\),\(3,'site_name','Eaton DC 1',1,0\),\(4,'address','Place de la Gare',1,0\),\(5,'contact_name',' 1345 Le Lieu',1,0\),\(6,'region','EMEA',1,0\),\(7,'contact_email','John Smith',1,0\),\(8,'maximum_number_racks','476010203',1,0\),\(9,'country','Switzerland',1,0\),\(10,'contact_phone','john.smith@company.com',1,0\),\(11,'description','main room',2,0\),\(12,'description','Annex',3,0\),\(13,'description','cage 01',4,0\),\(14,'type','cage',4,0\),\(15,'description','row in a cage',5,0\),\(16,'maximum_number_racks','10',5,0\),\(17,'description','row in a room',6,0\),\(18,'maximum_number_racks','10',6,0\),\(19,'description','rack 01',7,0\),\(20,'u_size','42U',7,0\),\(21,'serial_no','21545212HGFV2',7,0\),\(22,'model','RESSPU4210KB 600mm x 1000mm - 42U Rack',7,0\),\(23,'manufacturer','Cooper',7,0\),\(24,'description','rack Lab',8,0\),\(25,'u_size','42U',8,0\),\(26,'model','RESSPU4210KB 600mm x 1000mm - 42U Rack',8,0\),\(27,'manufacturer','Cooper',8,0\),\(29,'description','customer X \(IT\)',10,0\),\(30,'type','customer',10,0\),\(31,'description','MAIN 240V',11,0\),\(32,'description','Genset',12,0\),\(33,'maintenance_date','4-Jun-14',12,0\),\(34,'runtime','8',12,0\),\(35,'service_contact_name','Bob Jones',12,0\),\(36,'serial_no','G789456',12,0\),\(37,'maintenance_due','4-Sep-14',12,0\),\(38,'installation_date','4-Jun-14',12,0\),\(39,'model','45kW NG/LG 240V 3 Phase',12,0\),\(40,'service_contact_mail','Bob.Jones@company.com',12,0\),\(41,'service_contact_phone','476010203',12,0\),\(42,'manufacturer','Generac',12,0\),\(43,'description','Genset',13,0\),\(44,'maintenance_date','4-Jun-14',13,0\),\(45,'runtime','8',13,0\),\(46,'service_contact_name','Bob Jones',13,0\),\(47,'maintenance_due','4-Sep-14',13,0\),\(48,'installation_date','4-Jun-14',13,0\),\(49,'model','45kW NG/LG 240V 3 Phase',13,0\),\(50,'service_contact_mail','Bob.Jones@company.com',13,0\),\(51,'service_contact_phone','476010203',13,0\),\(52,'manufacturer','Generac',13,0\),\(53,'description','Genset',14,0\),\(54,'maintenance_date','4-Jun-14',14,0\),\(55,'runtime','8',14,0\),\(56,'service_contact_name','Bob Jones',14,0\),\(57,'location_w_pos','left',14,0\),\(58,'maintenance_due','4-Sep-14',14,0\),\(59,'installation_date','4-Jun-14',14,0\),\(60,'model','45kW NG/LG 240V 3 Phase',14,0\),\(61,'service_contact_mail','Bob.Jones@company.com',14,0\),\(62,'service_contact_phone','476010203',14,0\),\(63,'manufacturer','Generac',14,0\),\(64,'description','Automatic transfer switch',15,0\),\(65,'serial_no','ATC1235',15,0\),\(66,'model','ATC-100',15,0\),\(67,'manufacturer','eaton',15,0\),\(78,'description','SRV1 PRIMERGY RX100 G8',18,0\),\(79,'location_u_pos','6',18,0\),\(80,'u_size','1U',18,0\),\(81,'hostname.1','srv1.Bios.Labo.Kalif.com',18,0\),\(82,'end_warranty_date','4-Jun-14',18,0\),\(83,'installation_date','4-Jun-14',18,0\),\(84,'http_link.1','htps://srv1.Bios.Labo.Kalif.com',18,0\),\(85,'model','RX100 G8',18,0\),\(86,'ip.1','10.130.32.16',18,0\),\(87,'manufacturer','Fujisu',18,0\)"

I=$(do_dumpdb t_bios_asset_ext_attributes | grep INSERT | egrep -c "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_ext_attributes FAILED'"
echo 'Test of the table t_bios_asset_ext_attributes		PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

ELEMENT="\(3,'epdu'\),\(2,'genset'\),\(6,'feed'\),\(10,'N_A'\),\(4,'pdu'\),\(5,'server'\),\(9,'storage'\),\(7,'sts'\),\(8,'switch'\),\(1,'ups'\)"
I=$(do_dumpdb t_bios_asset_device_type | grep INSERT | egrep -c "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_device_type FAILED'"
echo 'Test of the table t_bios_asset_device_type		PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"
#ELEMENT="\(1,10,NULL,14,NULL,1\),\(2,12,NULL,14,NULL,1\)"
ELEMENT="\(1,10,NULL,13,NULL,1\),\(2,11,NULL,13,NULL,1\)"
I=$(do_dumpdb t_bios_asset_link | grep INSERT | egrep "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_link FAILED'"
echo 'Test of the table t_bios_asset_ext_link			PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

#ELEMENT="\(1,'power chain'\)"
ELEMENT="\(1,'power chain'\)"
I=$(do_dumpdb t_bios_asset_link_type | grep INSERT | egrep -c "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_link_type FAILED'"
echo 'Test of the table t_bios_asset_ext_link_type		PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

ELEMENT="\(1,4,5\)"
I=$(do_dumpdb t_bios_asset_group_relation | grep INSERT | egrep -c "$ELEMENT") 2>/dev/null
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_group_relation FAILED'"
echo 'Test of the table t_bios_asset_ext_group_relation       PASSED' | tee -a "$CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log"

TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Finish time is $TIME"
TIME_END=$(date +%s)
TEST_LAST=$(expr $TIME_END - $TIME_START)
echo "Test lasted $TEST_LAST second(s)."
exit 0
