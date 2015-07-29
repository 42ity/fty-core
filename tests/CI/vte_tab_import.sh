#!/bin/bash

# Copyright (C) 2014 Eaton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#   
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    # *** tests creates initial (near to empty assets) using tools/initdb.sql file.
    # *** tests compares the exported tableis files with expected (only INSERT command)

# ***** PREREQUISITES *****
    # *** SUT_SSH_PORT should be passed as parameter -sp <value>
    # *** it is currently from interval <2206;2209>
    # *** must run as root without using password 
    # *** BIOS image must be installed and running on SUT 
    # *** tools directory containing tools/initdb.sql tools/bam_vte_tab_import.csv present on MS 
    # *** tests/CI directory (on MS) contains weblib.sh and scriptlib.sh library files

# ***** GLOBAL VARIABLES *****
TIME_START=$(date +%s)

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
        *)  echo "$0: Unknown param and all after it are ignored: $@"
            break
            ;;
    esac
done

# default values:
[ -z "$SUT_USER" ] && SUT_USER="root"
[ -z "$SUT_HOST" ] && SUT_HOST="debian.roz.lab.etn.com"
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
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"
SUT_IS_REMOTE=yes

    # *** if used set BIOS_USER and BIOS_PASSWD for tests where it is used:
[ -z "$BIOS_USER" ] && BIOS_USER="bios"
[ -z "$BIOS_PASSWD" ] && BIOS_PASSWD="@PASSWORD@"

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "CI-FATAL: $0: Can not web script library" >&2; exit 1; }

logmsg_info "Will use BASE_URL = '$BASE_URL'"

determineDirs_default || true
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"

logmsg_info "Ensuring that needed remote daemons are running on VTE"
sut_run 'systemctl daemon-reload; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl start $SVC ; done'
sleep 5
sut_run 'R=0; for SVC in saslauthd malamute mysql bios-agent-dbstore bios-server-agent  bios-agent-nut bios-agent-inventory ; do systemctl status $SVC >/dev/null 2>&1 && echo "OK: $SVC" || { R=$?; echo "FAILED: $SVC"; }; done; exit $R' || \
    die "Some required services are not running on the VTE"

# ***** INIT DB *****
    # *** write power rack base test data to DB on SUT
set -o pipefail 2>/dev/null || true
set -e
loaddb_file ./tools/initdb.sql 2>&1 | tee $CHECKOUTDIR/vte-tab-${_SCRIPT_NAME}.log
set +e

# ***** POST THE CSV FILE *****
ASSET="$CHECKOUTDIR/tools/bam_vte_tab_import.csv"

# Import the bam_vte_tab_import.csv file
api_auth_post_file /asset/import assets=@$ASSET -H "Expect:" | tee $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

grep -q '"imported_lines" : 19' $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log || \
    die "ERROR : 'Test of the number of imported lines			FAILED'"
echo "Test of the number of imported lines			PASSED"

for NUM in 9 10 17 21 22 23 ; do
    grep -q "\[ $NUM," $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log || \
        die "ERROR : 'Test of the line   $NUM 				FAILED'"
    echo "Test of the line  $NUM  					PASSED"
done

#ELEMENT="\([0123456789]*,'DC-LAB1',2,NULL,'active',1,1\),\([0123456789]*,'ROOM-01',3,1,'active',2,0\),\([0123456789]*,'ANNEX-01',3,1,'active',3,1\),\([0123456789]*,'CAGE-01',1,2,'active',4,1\),\([0123456789]*,'ROW-01',4,2,'active',5,1\),\([0123456789]*,'ROW-02',4,2,'active',5,1\),\([0123456789]*,'RACK-01',5,5,'active',5,1\),\([0123456789]*,'RACK1-LAB',5,5,'active',5,0\),\([0123456789]*,'CUSTOMER_02',1,1,'nonactive',2,1\),\([0123456789]*,'MAIN_LAB',6,1,'nonactive',3,1\),\([0123456789]*,'MAIN3P_LAB',6,1,'nonactive',4,0\),\([0123456789]*,'GENSET_01',6,3,'spare',5,1\),\([0123456789]*,'GENSET_02',6,3,'retired',5,1\),\([0123456789]*,'GENSET_03',6,3,'nonactive',5,1\),\([0123456789]*,'ATS_01',6,2,'active',5,1\),\([0123456789]*,'UPS1-MAIN',6,2,'active',1,1\),\([0123456789]*,'ePDU1-LAB',6,8,'active',5,1\),\([0123456789]*,'SRV1_LAB',6,8,'active',5,1\),\([0123456789]*,'SRV2_LAB',6,8,'active',5,1\)"

ELEMENT="\(1,'DC-LAB1',2,NULL,'active',1,1\),\(2,'ROOM-01',3,1,'active',2,0\),\(3,'ANNEX-01',3,1,'active',3,1\),\(4,'CAGE-01',1,2,'active',4,1\),\(5,'ROW-01',4,2,'active',5,1\),\(6,'ROW-02',4,2,'active',5,1\),\(7,'RACK-01',5,5,'active',5,1\),\(8,'RACK1-LAB',5,5,'active',5,0\),\(10,'CUSTOMER_02',1,1,'nonactive',2,1\),\(11,'MAIN_LAB',6,1,'nonactive',3,1\),\(12,'MAIN3P_LAB',6,1,'nonactive',4,0\),\(13,'GENSET_01',6,3,'spare',5,1\),\(14,'GENSET_02',6,3,'retired',5,1\),\(15,'GENSET_03',6,3,'nonactive',5,1\),\(16,'ATS_01',6,2,'active',5,1\),\(17,'UPS1-MAIN',6,2,'active',1,1\),\(18,'ePDU1-LAB',6,8,'active',5,1\),\(20,'SRV1_LAB',6,8,'active',5,1\),\(21,'SRV2_LAB',6,8,'active',5,1\)"

I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_element|awk "/INSERT/ && /ROOM-01/" | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_element FAILED'"
echo 'Test of the table t_bios_asset_element 			PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(2,'datacenter'\),\(6,'device'\),\(1,'group'\),\(5,'rack'\),\(3,'room'\),\(4,'row'\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_element_type|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_element_type FAILED'"
echo 'Test of the table t_bios_asset_element_type 		PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="VALUES \(1,'company','EATON',1,0\),\(2,'description','Lab DC',1,0\),\(3,'site_name','Eaton DC 1',1,0\),\(4,'address','Place de la Gare',1,0\),\(5,'contact_name',' 1345 Le Lieu',1,0\),\(6,'region','EMEA',1,0\),\(7,'contact_email','John Smith',1,0\),\(8,'maximum_number_racks','476010203',1,0\),\(9,'country','Switzerland',1,0\),\(10,'contact_phone','john.smith@company.com',1,0\),\(11,'description','main room',2,0\),\(12,'description','Annex',3,0\),\(13,'description','cage 01',4,0\),\(14,'type','cage',4,0\),\(15,'description','row in a cage',5,0\),\(16,'maximum_number_racks','10',5,0\),\(17,'description','row in a room',6,0\),\(18,'maximum_number_racks','10',6,0\),\(19,'description','rack 01',7,0\),\(20,'u_size','42U',7,0\),\(21,'serial_no','21545212HGFV2',7,0\),\(22,'asset_tag','EATON123455',7,0\),\(23,'model','RESSPU4210KB 600mm x 1000mm - 42U Rack',7,0\),\(24,'manufacturerbrand','Cooper',7,0\),\(25,'description','rack Lab',8,0\),\(26,'u_size','42U',8,0\),\(27,'serial_no','21545212HGFV2',8,0\),\(28,'asset_tag','EATON123456',8,0\),\(29,'model','RESSPU4210KB 600mm x 1000mm - 42U Rack',8,0\),\(30,'manufacturerbrand','Cooper',8,0\),\(32,'description','customer X \(IT\)',10,0\),\(33,'type','customer',10,0\),\(34,'description','MAIN 240V',11,0\),\(35,'description','MAIN 380V 3Ph',12,0\),\(36,'phasefeeds','3',12,0\),\(37,'description','Genset',13,0\),\(38,'maintenance_date','4-Jun-14',13,0\),\(39,'service_contact_name','Bob Jones',13,0\),\(40,'serial_no','G789456',13,0\),\(41,'maintenance_due','4-Sep-14',13,0\),\(42,'runtimeautonomy','8',13,0\),\(43,'asset_tag','EATON123457',13,0\),\(44,'installation_date','4-Jun-14',13,0\),\(45,'model','45kW NG/LG 240V 3 Phase',13,0\),\(46,'service_contact_mail','Bob.Jones@company.com',13,0\),\(47,'service_contact_phone','476010203',13,0\),\(48,'manufacturerbrand','Generac',13,0\),\(49,'description','Genset',14,0\),\(50,'maintenance_date','4-Jun-14',14,0\),\(51,'service_contact_name','Bob Jones',14,0\),\(52,'serial_no','G789456',14,0\),\(53,'maintenance_due','4-Sep-14',14,0\),\(54,'runtimeautonomy','8',14,0\),\(55,'asset_tag','EATON123457',14,0\),\(56,'installation_date','4-Jun-14',14,0\),\(57,'model','45kW NG/LG 240V 3 Phase',14,0\),\(58,'service_contact_mail','Bob.Jones@company.com',14,0\),\(59,'service_contact_phone','476010203',14,0\),\(60,'manufacturerbrand','Generac',14,0\),\(61,'description','Genset',15,0\),\(62,'maintenance_date','4-Jun-14',15,0\),\(63,'service_contact_name','Bob Jones',15,0\),\(64,'location_w_pos','left',15,0\),\(65,'serial_no','G789456',15,0\),\(66,'maintenance_due','4-Sep-14',15,0\),\(67,'runtimeautonomy','8',15,0\),\(68,'asset_tag','EATON123457',15,0\),\(69,'installation_date','4-Jun-14',15,0\),\(70,'model','45kW NG/LG 240V 3 Phase',15,0\),\(71,'service_contact_mail','Bob.Jones@company.com',15,0\),\(72,'service_contact_phone','476010203',15,0\),\(73,'manufacturerbrand','Generac',15,0\),\(74,'description','Automatic transfer switch',16,0\),\(75,'serial_no','ATC1235',16,0\),\(76,'asset_tag','EATON123458',16,0\),\(77,'model','ATC-100',16,0\),\(78,'manufacturerbrand','eaton',16,0\),\(79,'description','UPS somehow',17,0\),\(80,'battery_maintenance_date','4-Jun-14',17,0\),\(81,'u_size','3U',17,0\),\(82,'maintenance_date','4-Jun-14',17,0\),\(83,'battery_installation_date','4-Jun-14',17,0\),\(84,'serial_no','G214D17012',17,0\),\(85,'end_warranty_date','4-Jun-14',17,0\),\(86,'battery_type','4-Jun-14',17,0\),\(87,'asset_tag','EATON123459',17,0\),\(88,'installation_date','4-Jun-14',17,0\),\(89,'model','9PX 6kVA',17,0\),\(90,'manufacturerbrand','EATON',17,0\),\(91,'description','ePDU1 eMAA10',18,0\),\(92,'maintenance_date','4-Jun-14',18,0\),\(93,'location_w_pos','left',18,0\),\(94,'serial_no','EATON123460',18,0\),\(95,'hostname.1hostname','10.130.32.15',18,0\),\(96,'runtimeautonomy','U051B43007',18,0\),\(97,'battery_type','4-Jun-14',18,0\),\(98,'asset_tag',' 3xC19',18,0\),\(99,'http_link.1','MApdu-BiosLeft.Bios.Labo.Kalif.com',18,0\),\(100,'model','Eaton ePDU MA 1P IN:IEC309 16A OUT:21xC13',18,0\),\(101,'manufacturerbrand','EATON',18,0\),\(113,'description','SRV1 PRIMERGY RX100 G8',20,0\),\(114,'ip.1ip','10.130.32.16',20,0\),\(115,'location_u_pos','6',20,0\),\(116,'u_size','1U',20,0\),\(117,'serial_no','G789456',20,0\),\(118,'hostname.1hostname','srv1.Bios.Labo.Kalif.com',20,0\),\(119,'end_warranty_date','4-Jun-14',20,0\),\(120,'asset_tag','EATON123461',20,0\),\(121,'installation_date','4-Jun-14',20,0\),\(122,'http_link.1','htps://srv1.Bios.Labo.Kalif.com',20,0\),\(123,'model','RX100 G8',20,0\),\(124,'manufacturerbrand','Fujisu',20,0\),\(125,'description','SRV1 PRIMERGY RX100 G8',21,0\),\(126,'ip.1ip','10.130.32.16',21,0\),\(127,'location_u_pos','6',21,0\),\(128,'u_size','1U',21,0\),\(129,'serial_no','G789456',21,0\),\(130,'hostname.1hostname','srv1.Bios.Labo.Kalif.com',21,0\),\(131,'end_warranty_date','4-Jun-14',21,0\),\(132,'asset_tag','EATON123461',21,0\),\(133,'installation_date','4-Jun-14',21,0\),\(134,'http_link.1','htps://srv1.Bios.Labo.Kalif.com',21,0\),\(135,'model','RX100 G8',21,0\),\(136,'manufacturerbrand','Fujisu',21,0\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_ext_attributes|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_ext_attributes FAILED'"
echo 'Test of the table t_bios_asset_ext_attributes		PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(1,11,6\),\(2,12,6\),\(3,13,2\),\(4,14,1\),\(5,15,4\),\(6,16,7\),\(7,17,1\),\(8,18,3\),\(9,20,5\),\(10,21,5\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_device|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_device FAILED'"
echo 'Test of the table t_bios_asset_device			PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(3,'epdu'\),\(2,'genset'\),\(6,'main'\),\(4,'pdu'\),\(5,'server'\),\(7,'sts'\),\(1,'ups'\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_device_type|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_device_type FAILED'"
echo 'Test of the table t_bios_asset_device_type		PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(1,1,NULL,6,NULL,1\),\(2,3,NULL,6,NULL,1\),\(3,6,NULL,7,NULL,1\),\(4,7,NULL,8,NULL,1\),\(5,8,'B8',9,NULL,1\),\(6,8,'B9',10,NULL,1\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_link|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_link FAILED'"
echo 'Test of the table t_bios_asset_ext_link			PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(1,'power chain'\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_link_type|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_link_type FAILED'"
echo 'Test of the table t_bios_asset_ext_link_type		PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log

ELEMENT="\(1,4,5\)"
I=$(sut_run 'mysqldump -u root box_utf8 t_bios_asset_group_relation|grep INSERT | egrep "'"$ELEMENT"'"|wc -l' 2>/dev/null)
[ "$I" = 1 ] || die "ERROR : 'Test of the table t_bios_asset_group_relation FAILED'"
echo 'Test of the table t_bios_asset_ext_group_relation       PASSED' | tee -a $CHECKOUTDIR/DC008-${_SCRIPT_NAME}.log
TIME=$(date --utc "+%Y-%m-%d %H:%M:%S")
echo "Finish time is $TIME"
TIME_END=$(date +%s)
TEST_LAST=$(expr $TIME_END - $TIME_START)
echo "Test lasted $TEST_LAST second."
exit 0
