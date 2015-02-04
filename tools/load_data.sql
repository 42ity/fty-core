use box_utf8;

/* Constants */

/* t_bios_asset_device_type */
select @asset_device_ups := id_asset_device_type from t_bios_asset_device_type where name = 'ups'; 
select @asset_device_epdu := id_asset_device_type from t_bios_asset_device_type where name = 'epdu';
select @asset_device_server := id_asset_device_type from t_bios_asset_device_type where name = 'server';
select @asset_device_main := id_asset_device_type from t_bios_asset_device_type where name = 'main';

/* t_bios_device_type */
select @device_unclassified := id_device_type from t_bios_device_type where name = 'not_classified';
select @device_ups := id_device_type from t_bios_device_type where name = 'ups';
select @device_epdu := id_device_type from t_bios_device_type where name = 'epdu';
select @device_pdu := id_device_type from t_bios_device_type where name = 'pdu';
select @device_server := id_device_type from t_bios_device_type where name = 'server';

/* t_bios_asset_element_type */
select @asset_element_group := id_asset_element_type from t_bios_asset_element_type where name ='group';
select @asset_element_datacenter := id_asset_element_type from t_bios_asset_element_type where name ='datacenter';
select @asset_element_room := id_asset_element_type from t_bios_asset_element_type where name ='room';
select @asset_element_row := id_asset_element_type from t_bios_asset_element_type where name ='row';
select @asset_element_rack := id_asset_element_type from t_bios_asset_element_type where name ='rack';
select @asset_element_device := id_asset_element_type from t_bios_asset_element_type where name ='device';

/* t_bios_asset_link_type; */
select @asset_link_powerchain := id_asset_link_type from t_bios_asset_link_type where name = 'power chain';

/* t_bios_measurement_types */
select @measurement_temperature := id from t_bios_measurement_types where name = 'temperature';
select @measurement_voltage := id from t_bios_measurement_types where name = 'voltage';

/* t_bios_client */
select @client_nmap := id_client from t_bios_client where name = 'nmap';
select @client_mymodule := id_client from t_bios_client where name = 'mymodule';
select @client_admin := id_client from t_bios_client where name = 'admin';
select @client_nut := id_client from t_bios_client where name = 'NUT';

/*  Data  */

insert into t_bios_discovered_device (name, id_device_type) values ("select_device", @device_unclassified);
insert into t_bios_discovered_device (name, id_device_type) values ("monitor_asset_measure", @device_unclassified);
SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "select_device";
SELECT @monitor_asset_measure := id_discovered_device FROM t_bios_discovered_device WHERE name = "monitor_asset_measure";

/* Total rack Power tests */
INSERT INTO t_bios_discovered_device (name, id_device_type) values ("test_rc_pwr_epdu1", @device_epdu);
insert into t_bios_discovered_device (name, id_device_type) values ("test_rc_pwr_epdu2", @device_epdu);
SELECT @test_rc_pwr_epdu1 := id_discovered_device FROM t_bios_discovered_device WHERE name = "test_rc_pwr_epdu1";
SELECT @test_rc_pwr_epdu2 := id_discovered_device FROM t_bios_discovered_device WHERE name = "test_rc_pwr_epdu2";

/* NOTE: subquery is current mysql Error 1093 workaround */
insert into t_bios_asset_element (name, id_type, id_parent) values ("DC1",     @asset_element_datacenter, NULL);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROOM1",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROW1",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "RACK1",
    @asset_element_rack,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROW1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "serv1",
    @asset_element_device,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'RACK1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "epdu",
    @asset_element_device,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ups",
    @asset_element_device,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "main",
    @asset_element_device,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC1') as subquery)
);
insert into t_bios_asset_element (name, id_type, id_parent) values ("group1",  @asset_element_group,     NULL);
insert into t_bios_asset_element (name, id_type, id_parent) values ("DC2",     @asset_element_datacenter, NULL);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROOM2",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROOM3",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
);
insert into t_bios_asset_element (name, id_type, id_parent) values ("ROOM4",   @asset_element_room,      NULL);
insert into t_bios_asset_element (name, id_type, id_parent) values ("ROW4",    @asset_element_row,       NULL);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROW2",
    @asset_element_row, 
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM2') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROW3", 
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM3') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values 
(
    "ROW5",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM3') as subquery)
);
insert into t_bios_asset_element
    (name, id_type, id_parent)
values
(
    "ROW6",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
);

insert into t_bios_asset_device
    (id_asset_element, id_asset_device_type, mac)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'serv1'),
    @asset_device_server,
    "11:22:33:44:55:66"
);
insert into t_bios_asset_device
    (id_asset_element, id_asset_device_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'epdu'),
    @asset_device_epdu
);
insert into t_bios_monitor_asset_relation
    (id_discovered_device, id_asset_element)
values
(
    (select id_discovered_device from t_bios_discovered_device where name = 'select_device' AND id_device_type = @device_unclassified),
    (select id_asset_element from t_bios_asset_element where name = 'ups')
);
insert into t_bios_monitor_asset_relation
    (id_discovered_device, id_asset_element)
values
(
    (select id_discovered_device from t_bios_discovered_device where name = 'monitor_asset_measure' AND id_device_type = @device_unclassified),
    (select id_asset_element from t_bios_asset_element where name = 'epdu')
);
insert into t_bios_asset_device
    (id_asset_element, id_asset_device_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'ups'),
    @asset_device_ups
);
insert into t_bios_asset_device
    (id_asset_element, id_asset_device_type)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'main'),
    @asset_device_main
);

/* t_bios_asset_group_relation ('DC1', 'epdu') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'epdu')
);
/* t_bios_asset_group_relation ('DC1', 'ups') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'ups')
);
/* t_bios_asset_group_relation ('DC1', 'main') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'main')
);

insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'main')
           AND id_asset_device_type = @asset_device_main), /* 4 */
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'ups')
           AND id_asset_device_type = @asset_device_ups), /* 3 */
    @asset_link_powerchain /* 1 */,
    1,
    2
);
/* ( t_bios_asset_device ('ups' 'ups');  */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'ups')
           AND id_asset_device_type = @asset_device_ups),
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'epdu')
           AND id_asset_device_type = @asset_device_epdu),
    @asset_link_powerchain
);
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'epdu')
            AND id_asset_device_type = @asset_device_epdu),
    (select id_asset_device from t_bios_asset_device
     where id_asset_element = (select id_asset_element from t_bios_asset_element where name = 'serv1')
            AND id_asset_device_type = @asset_device_server AND mac = '11:22:33:44:55:66'),
    @asset_link_powerchain
);

insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "total_facility_load",
    "2",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "contact",
    "mike@nn.com",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "description",
    "room is very cute",
    (select id_asset_element from t_bios_asset_element where name = 'ROOM4')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "flowers",
    "yes, in this room there are only flowers",
    (select id_asset_element from t_bios_asset_element where name = 'ROOM4')
);

INSERT INTO t_bios_discovered_device (id_device_type, name) values (@device_unclassified, "measures");
SELECT @measures_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "measures";

/* voltage.output */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    1,
    1,
    32,
    "2014-11-12 09:45:59",
    @client_nut,
    @select_device
);

/* voltage.output */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    1,
    1,
    3,
    "2014-11-12 09:46:59",
    @client_nut,
    @select_device
);

/* current.output */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    2,
    1,
    31,
    "2014-11-12 09:47:59",
    @client_nut,
    @select_device
);

/* current.output.L1 */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    2,
    2,
    12,
    "2014-11-12 09:48:59",
    @client_nut,
    @select_device
);

/* voltage.output.L1-N */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    1,
    2,
    1,
    "2014-11-12 09:49:59",
    @client_nut,
    @select_device
);

/* realpower.output */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    1,
    1,
    "2014-11-12 09:59:59",
    @client_nut,
    @select_device
);

/* status.ups */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    7,
    1,
    2,
    "2014-11-12 09:59:59",
    @client_nut,
    @select_device
);

/* temperature.default */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    4,
    1,
    56,
    "2014-11-12 09:59:59",
    @client_nut,
    @select_device
);

/* load.ups */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    5,
    1,
    17,
    "2014-11-12 09:59:59",
    @client_nut,
    @select_device
);

/* charge.battery */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    6,
    1,
    931,
    "2014-11-12 09:59:59",
    @client_nut,
    @select_device
);

/* epdu */

/* realpower.outlet */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    5,
    2405,
    "2014-11-12 09:59:59",
    @client_nut,
    @monitor_asset_measure
);

/* realpower.outlet.1 */
SELECT @realpower_outlet_1 := id FROM t_bios_measurement_subtypes WHERE name = "outlet.1";
SELECT @realpower_outlet_2 := id FROM t_bios_measurement_subtypes WHERE name = "outlet.2";
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    @realpower_outlet_1,
    2405,
    "2014-11-12 09:59:59",
    @client_nut,
    @monitor_asset_measure
);

/* realpower.outlet.2 */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    @realpower_outlet_2,
    500,
    "2014-11-12 09:59:59",
    @client_nut,
    @monitor_asset_measure
);

/* ### test total rack power ### */
/* realpower.outlet */
insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    5,
    2405,
    UTC_TIMESTAMP(),
    @client_nut,
    @test_rc_pwr_epdu1
);

insert into t_bios_measurements
    (id_type, id_subtype, value, timestamp, id_client, id_device)
values
(
    3,
    5,
    2405,
    DATE_SUB(UTC_TIMESTAMP(), INTERVAL 60 MINUTE),
    @client_nut,
    @test_rc_pwr_epdu2
);


INSERT INTO t_bios_net_history (command, ip, mask, mac, name, timestamp) VALUES ("a", "fe80", 64, "wlo1", "c4:d9:87:2f:dc:7b", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (command, ip, mask, mac, name, timestamp) VALUES ("m", "192.168.1.0", 24, "", "", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (command, ip, mask, mac, name, timestamp) VALUES ("a", "10.231.107.0", 24, "enp0s25", "a0:1d:48:b7:e2:4e", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (command, ip, mask, mac, name, timestamp) VALUES ("d", "10.0.0.0", 8, "", "", UTC_TIMESTAMP());

/* ************* */
/* MBT Rack data */
/* ************* */

/* DC-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("DC-LAB", @asset_element_datacenter,  NULL);
set @last_asset_element := LAST_INSERT_ID();
set @last_datacenter := @last_asset_element; 
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",    "EATON Montobonnot Datacenter", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("company",    "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("site",       "Montbonnot", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("country",    "France", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("address",    "110 Rue Blaise Pascal, 38330 Montbonnot-Saint-Martin", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact",    "john.smith@eaton.com", @last_asset_element);

/* ROOM1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM1-LAB", @asset_element_room, @last_datacenter);
set @last_asset_element := LAST_INSERT_ID();
set @last_room := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "Lab Room", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("floor", "0", @last_asset_element);

/* RACK1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "RACK1-LAB", @asset_element_rack, @last_room);
set @last_asset_element := LAST_INSERT_ID();
set @last_rack := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "BIOS Rack Validation", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("free_space",  "27",      @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand",       "EATON",   @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("type",        "1",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("height",      "27U",     @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("width",       "600",     @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("depth",       "800",     @last_asset_element);

/* UPS1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("UPS1-LAB", @asset_element_device, @last_rack);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("end_warranty_date", "2018-31-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("warranty_expiration_date", "2020-31-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_warranty_expiration_date", "2019-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_maintenance_date", "2016-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "2",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "0",       @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_ups);

/* UPS2-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("UPS2-LAB", @asset_element_device, @last_rack);
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS2 9PX 6kVA", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("end_warranty_date", "2018-31-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("warranty_expiration_date", "2020-31-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_warranty_expiration_date", "2019-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_maintenance_date", "2016-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "2",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "2",       @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_ups);

/* ePDU1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("ePDU1-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU1 Marlin", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_epdu);

/* ePDU2-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("ePDU2-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU2 Marlin", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_epdu);

/* SRV1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV1-LAB",  @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV1 HP",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "2",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "10",       @last_asset_element);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (NULL, @last_asset_element, @asset_device_server);

/* SRV2-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV2-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV2 HP", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "2",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "12",       @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_server);

/* MAIN-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("MAIN-LAB", @asset_element_device, @last_datacenter);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "MAIN 240V", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("feeds",       "2",         @last_asset_element);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (NULL, @last_asset_element, @asset_device_main);

/* GROUP1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("GROUP1-LAB", @asset_element_group, @last_datacenter);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "input power chain", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("type",        "input_power",       @last_asset_element);

/* Create a group */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from `t_bios_asset_element` where name = 'GROUP1-LAB'),
    (select id_asset_element from `t_bios_asset_element` where name = 'MAIN-LAB')
);

/* Asset links */

/* link (MAIN-LAB, UPS1-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'MAIN-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS1-LAB'),
    @asset_link_powerchain
);
/* link (MAIN-LAB, UPS2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'MAIN-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS2-LAB'),
    @asset_link_powerchain
);
/* link (UPS1-LAB, ePDU1-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    @asset_link_powerchain
);
/* link (UPS2-LAB, ePDU2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    @asset_link_powerchain
);
/* link (ePDU1-LAB, SRV1-LAB, 'power chain', 10, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    10, 
    1
);
/* link (ePDU2-LAB, SRV1-LAB, 'power chain', 10, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    10,
    2
);
/* link (ePDU1-LAB, SRV2-LAB, 'power chain', 11, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    11,
    1
);
/* link (ePDU2-LAB, SRV2-LAB, 'power chain', 11, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    11,
    2
);

SELECT @client_ui_properties := id_client FROM t_bios_client WHERE name = 'ui_properties';
UPDATE t_bios_client_info SET timestamp=UTC_TIMESTAMP(), ext='{"key1" : "value1", "key2" : "value2"}' WHERE id_client=@client_ui_properties;


/* A manual propagate an asset part to monitor part*/
/* UPSs */
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('UPS1-LAB', @device_ups);
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('UPS2-LAB', @device_ups);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'UPS1-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'UPS1-LAB')
);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'UPS2-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'UPS2-LAB')
);

/* ePDUs */
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('ePDU1-LAB', @device_epdu);
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('ePDU2-LAB', @device_epdu);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'ePDU1-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'ePDU1-LAB')
);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'ePDU2-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'ePDU2-LAB')
);

/* SRVs */
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('SRV1-LAB', @device_server);
INSERT INTO t_bios_discovered_device (name,id_device_type) VALUES ('SRV2-LAB', @device_server);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'SRV1-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'SRV1-LAB')
);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) VALUES (
    (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'SRV2-LAB'),
    (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'SRV2-LAB')
);


