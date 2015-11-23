use box_utf8;
/* t_bios_asset_element_type */
select @asset_element_group := id_asset_element_type from t_bios_asset_element_type where name ='group';
select @asset_element_datacenter := id_asset_element_type from t_bios_asset_element_type where name ='datacenter';
select @asset_element_room := id_asset_element_type from t_bios_asset_element_type where name ='room';
select @asset_element_row := id_asset_element_type from t_bios_asset_element_type where name ='row';
select @asset_element_rack := id_asset_element_type from t_bios_asset_element_type where name ='rack';
select @asset_element_device := id_asset_element_type from t_bios_asset_element_type where name ='device';

/* t_bios_asset_device_type */
select @asset_device_ups := id_asset_device_type from t_bios_asset_device_type where name = 'ups'; 
select @asset_device_epdu := id_asset_device_type from t_bios_asset_device_type where name = 'epdu';
select @asset_device_server := id_asset_device_type from t_bios_asset_device_type where name = 'server';
select @asset_device_feed := id_asset_device_type from t_bios_asset_device_type where name = 'feed';

/* t_bios_asset_link_type; */
select @asset_link_powerchain := id_asset_link_type from t_bios_asset_link_type where name = 'power chain';

/* t_bios_device_type */
select @device_unclassified := id_device_type from t_bios_device_type where name = 'not_classified';
select @device_ups := id_device_type from t_bios_device_type where name = 'ups';
select @device_epdu := id_device_type from t_bios_device_type where name = 'epdu';
select @device_pdu := id_device_type from t_bios_device_type where name = 'pdu';
select @device_server := id_device_type from t_bios_device_type where name = 'server';

/* ************* */
/* outage Rack data */
/* ************* */

/* outage-LAB */
insert into t_bios_asset_element (name , id_type, id_parent,status,priority,business_crit) values ("DC-outage", @asset_element_datacenter,  NULL,"active",1,1);
select @last_asset_element:=id_asset_element from t_bios_asset_element where name="DC-outage";
set @last_datacenter := @last_asset_element; 
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",    "EEIC Roztoky DC", @last_asset_element);
/* outage.ROOM01 */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent,status,priority,business_crit) values (NULL, "outage.ROOM01", @asset_element_room, @last_datacenter,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
set @last_room := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "IT Server room", @last_asset_element);

/* outage.ROOM01.RACK01 */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent,status,priority,business_crit) values (NULL, "outage.ROOM01.RACK01", @asset_element_rack, @last_room,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
set @last_rack := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("model", "RESSPU4210KB 600mm x 1000mm - 42U Rack", @last_asset_element);

/* outage.UPS1 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.UPS1", @asset_element_device, @asset_device_ups, @last_rack,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);

/* outage.UPS2 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.UPS2", @asset_element_device, @asset_device_ups, @last_rack,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);

/* outage.UPS3 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.UPS3", @asset_element_device, @asset_device_ups, @last_rack,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);

/* outage.UPS4 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.UPS4", @asset_element_device, @asset_device_ups, @last_rack,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);

/* outage.UPS5 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.UPS5", @asset_element_device, @asset_device_ups, @last_rack,"active",1,1);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);

/* outage.ePDU05 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.ePDU05", @asset_element_device, @asset_device_epdu, @last_rack,"active",1,1);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU05 eSWA01", @last_asset_element);

/* outage.ePDU04 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("outage.ePDU04", @asset_element_device, @asset_device_epdu, @last_rack,"active",1,1);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU04 eMAA10", @last_asset_element);

/* GRASSHOPPER */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent,status,priority,business_crit) values ("GRASSHOPPER",  @asset_element_device, @asset_device_server, @last_rack,"active",1,1);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "Secondary BIOS server (kvm)",  @last_asset_element);

/* SRV13 */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("SRV13", @asset_element_device, @asset_device_server, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","IT Power testing server", @last_asset_element);

/* HORNET */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("HORNET",  @asset_element_device, @asset_device_server, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "Main BIOS server (kvm, OBS, CI)",  @last_asset_element);

/* SHERPA */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("SHERPA", @asset_element_device, @asset_device_server, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","BIOS NAS", @last_asset_element);

/* HP-UX */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("HP-UX", @asset_element_device, @asset_device_server, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","Testing machine with HP-UX", @last_asset_element);

/* BIOS-RC-DEMO */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("BIOS-RC-DEMO",  @asset_element_device, @asset_device_server, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "BIOS Rack Controller",  @last_asset_element);

/* outage.feed */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent) values ("outage.feed", @asset_element_device, @asset_device_feed, @last_datacenter);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "feed 240V", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("phase",       "1",         @last_asset_element);

/* Asset links */

/* link (outage.feed, outage.UPS1, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.feed'),
    (select id_asset_element from t_bios_asset_element where name = 'outage.UPS1'),
    @asset_link_powerchain
);

/* link (outage.UPS1, outage.ePDU05, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.UPS1'),
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    @asset_link_powerchain
);

/* link (outage.UPS1, outage.ePDU04, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.UPS1'),
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    @asset_link_powerchain
);

/* link (outage.ePDU05, HP-UX, 'power chain', A4, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'HP-UX'),
    @asset_link_powerchain,
    "3",
    "1"
);
/* link (outage.ePDU04, HP-UX, 'power chain', A4, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'HP-UX'),
    @asset_link_powerchain,
    "3",
    "2"
);

/* link (outage.ePDU05,GRASSHOPPER, 'power chain', A4, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'GRASSHOPPER'),
    @asset_link_powerchain,
    "4",
    "1"
);
/* link (outage.ePDU04, GRASSHOPPER, 'power chain', A4, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'GRASSHOPPER'),
    @asset_link_powerchain,
    "4",
    "2"
);
/* link (outage.ePDU05, SRV13, 'power chain', A5, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV13'),
    @asset_link_powerchain,
    "5",
    "1"
);
/* link (outage.ePDU04, SRV13, 'power chain', A5, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV13'),
    @asset_link_powerchain,
    "5",
    "2"
);

/* link (outage.ePDU05, HORNET, 'power chain', A6, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'HORNET'),
    @asset_link_powerchain,
    "6",
    "1"
);
/* link (outage.ePDU04, HORNET, 'power chain', A6, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'HORNET'),
    @asset_link_powerchain,
    "6",
    "2"
);

/* link (outage.ePDU05, SHERPA, 'power chain', A7, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'SHERPA'),
    @asset_link_powerchain,
    "7",
    "1"
);
/* link (outage.ePDU04, SHERPA, 'power chain', A7, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'SHERPA'),
    @asset_link_powerchain,
    "7",
    "2"
);

/* link (outage.ePDU05, BIOS-RC-DEMO, 'power chain', A8, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU05'),
    (select id_asset_element from t_bios_asset_element where name = 'BIOS-RC-DEMO'),
    @asset_link_powerchain,
    "8",
    "1"
);
/* link (outage.ePDU04, BIOS-RC-DEMO, 'power chain', A8, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'outage.ePDU04'),
    (select id_asset_element from t_bios_asset_element where name = 'BIOS-RC-DEMO'),
    @asset_link_powerchain,
    "8",
    "2"
);

/* A manual propagate an asset part to monitor part*/
/* UPSs */
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('outage.UPS1', @device_ups);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'outage.UPS1'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'outage.UPS1')
);

/* ePDUs */
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('outage.ePDU05', @device_epdu);
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('outage.ePDU04', @device_epdu);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'outage.ePDU05'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'outage.ePDU05')
);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'outage.ePDU04'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'outage.ePDU04')
);

INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-08 13:23:03') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-08 13:57:12') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-08 23:56:51') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 01:58:22') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS4', UNIX_TIMESTAMP('2015-06-08 06:56:51') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-10 06:58:22') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-09 06:56:51') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 06:58:22') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-09 11:38:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:40:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS2', UNIX_TIMESTAMP('2015-06-09 11:39:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:42:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS3', UNIX_TIMESTAMP('2015-06-09 11:40:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:50:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-09 11:49:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:55:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS3', UNIX_TIMESTAMP('2015-06-09 11:56:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:57:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS2', UNIX_TIMESTAMP('2015-06-09 11:58:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-09 11:59:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS5', UNIX_TIMESTAMP('2015-06-09 23:58:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-10 11:59:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-11 09:37:59') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-11 09:43:35') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-06-11 09:45:32') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-06-11 09:46:31') ,            0 , @last_datacenter );

INSERT INTO t_bios_alert ( rule_name, date_from, priority, state, description, date_till, notification, dc_id) VALUES
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-08 13:23:03') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-08 13:57:12') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-08 23:56:51') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 01:58:22') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-09 06:56:51') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 06:58:22') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-09 11:38:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:40:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS2', UNIX_TIMESTAMP('2015-07-09 11:39:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:42:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS3', UNIX_TIMESTAMP('2015-07-09 11:40:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:50:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-09 11:49:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:55:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS3', UNIX_TIMESTAMP('2015-07-09 11:56:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:57:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS2', UNIX_TIMESTAMP('2015-07-09 11:58:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-09 11:59:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS5', UNIX_TIMESTAMP('2015-07-09 23:58:29') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-10 11:59:08') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-11 09:37:59') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-11 09:43:35') ,            0 , @last_datacenter ),
(  'upsonbattery@outage.UPS1', UNIX_TIMESTAMP('2015-07-11 09:45:32') ,        1 ,     1 , 'UPS is running on battery!    ', UNIX_TIMESTAMP('2015-07-11 09:46:31') ,            0 , @last_datacenter );
