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

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet.2@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet_2 := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet.2@monitor_asset_measure";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet.1@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet_1 := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet.1@monitor_asset_measure";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet  := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet@monitor_asset_measure";


insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "", "status.ups@select_device" );
SELECT @test_topic_s_d_status_ups := id FROM t_bios_measurement_topic WHERE topic = "status.ups@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "%", "charge.battery@select_device" );
SELECT @test_topic_s_d_charge_battery := id FROM t_bios_measurement_topic WHERE topic = "charge.battery@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "%", "load.default@select_device" );
SELECT @test_topic_s_d_load_default := id FROM t_bios_measurement_topic WHERE topic = "load.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "C", "temperature.default@select_device" );
SELECT @test_topic_s_d_temperature_default := id FROM t_bios_measurement_topic WHERE topic = "temperature.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "W", "realpower.default@select_device" );
SELECT @test_topic_s_d_realpower_defaulf := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "A", "current.output.L1@select_device" );
SELECT @test_topic_s_d_current_output_L1 := id FROM t_bios_measurement_topic WHERE topic = "current.output.L1@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "A", "current.output@select_device" );
SELECT @test_topic_s_d_current_output := id FROM t_bios_measurement_topic WHERE topic = "current.output@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "V", "voltage.output.L1-N@select_device" );
SELECT @test_topic_s_d_voltage_otput_L1_N := id FROM t_bios_measurement_topic WHERE topic = "voltage.output.L1-N@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "V", "voltage.output@select_device" );
SELECT @test_topic_s_d_voltage_output := id FROM t_bios_measurement_topic WHERE topic = "voltage.output@select_device";


insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:59", 50, 0, @test_topic_m_a_m_realpower_outlet_2 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:58", 2405, -1, @test_topic_m_a_m_realpower_outlet_1 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:57", 2405, -1, @test_topic_m_a_m_realpower_outlet );


insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:59", 2, 0, @test_topic_s_d_status_ups );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:58", 9310, -2, @test_topic_s_d_charge_battery );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:57", 17, -1, @test_topic_s_d_load_default );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:59", 56, -1, @test_topic_s_d_temperature_default );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:59", 1000, -4, @test_topic_s_d_realpower_defaulf );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:58", 12, -1, @test_topic_s_d_current_output_L1 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:57", 31, -1, @test_topic_s_d_current_output );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:58", 10, -2, @test_topic_s_d_voltage_otput_L1_N );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, "2014-11-12 09:59:57", 3, -1, @test_topic_s_d_voltage_output );

/*
-{"current":[{"id":"6","name":"monitor_asset_measure","realpower.outlet.2":50.0,"realpower.outlet.1":240.5,"realpower.outlet":240.5}]}
-{"current":[{"id":"7","name":"select_device","status.ups":"TRIM","charge.battery":93.1,"load.default":1.7,"temperature.default":5.6,"realpower.default":0.1,"current.output.L1":1.2,"current.output":3.1,"voltage.output.L1-N":0.1,"voltage.output":0.3}]}
*/

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
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "3",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "0",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname", "9PX-BiosDown", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("full_hostname", "9PX-BiosDown.Bios.Labo.Kalif.com", @last_asset_element);
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
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "3",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "3",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname", "9PX-BiosUp", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("full_hostname", "9PX-BiosUp.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_ups);

/* ePDU1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("ePDU1-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU1 eMAA10", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname", "MApdu-BiosLeft", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("full_hostname", "MApdu-BiosLeft.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_epdu);

/* ePDU2-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("ePDU2-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "ePDU2 eMAA10", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname", "MApdu-BiosRight", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("full_hostname", "MApdu-BiosRight.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_epdu);

/* SRV1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV1-LAB",  @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV1 PRIMERGY RX100 G8",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "6",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "Fujitsu", @last_asset_element);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (NULL, @last_asset_element, @asset_device_server);

/* SRV2-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV2-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV2 PRIMERGY RX100 G8", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "7",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "Fujitsu", @last_asset_element);
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

/* SRV3-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV3-LAB",  @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV3 DL320e G8",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "8",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "HP", @last_asset_element);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (NULL, @last_asset_element, @asset_device_server);

/* SRV4-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV4-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV4 DL320e G8", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "9",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "HP", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_server);

/* SRV5-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV5-LAB",  @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV5 PowerEdge R320",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "11",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "Dell", @last_asset_element);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (NULL, @last_asset_element, @asset_device_server);

/* SRV6-LAB */
insert into t_bios_asset_element (name , id_type, id_parent) values ("SRV6-LAB", @asset_element_device, @last_rack);
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV6 System x 3530 M4", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_height",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "13",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("brand", "IBM", @last_asset_element);
insert into t_bios_asset_device  (id_asset_element, id_asset_device_type) values (@last_asset_element, @asset_device_server);

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
/* link (UPS1-LAB, ePDU2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    @asset_link_powerchain
);
/* link (UPS2-LAB, ePDU2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'UPS2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    @asset_link_powerchain
);
/* link (ePDU1-LAB, SRV1-LAB, 'power chain', B6, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    "B6", 
    "1"
);
/* link (ePDU2-LAB, SRV1-LAB, 'power chain', B6, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    "B6",
    "1"
);
/* link (ePDU1-LAB, SRV2-LAB, 'power chain', B5, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    "B5",
    "1"
);
/* link (ePDU2-LAB, SRV2-LAB, 'power chain', B5, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    "B5",
    "2"
);

/* link (ePDU1-LAB, SRV3-LAB, 'power chain', B2, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV3-LAB'),
    @asset_link_powerchain,
    "B2",
    "1"
);
/* link (ePDU2-LAB, SRV3-LAB, 'power chain', B2, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV3-LAB'),
    @asset_link_powerchain,
    "B2",
    "2"
);

/* link (ePDU1-LAB, SRV4-LAB, 'power chain', B1, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV4-LAB'),
    @asset_link_powerchain,
    "B1",
    "1"
);
/* link (ePDU2-LAB, SRV4-LAB, 'power chain', B1, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV4-LAB'),
    @asset_link_powerchain,
    "B1",
    "2"
);

/* link (ePDU1-LAB, SRV5-LAB, 'power chain', A7, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV5-LAB'),
    @asset_link_powerchain,
    "A7",
    "1"
);
/* link (ePDU2-LAB, SRV5-LAB, 'power chain', A7, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV5-LAB'),
    @asset_link_powerchain,
    "A7",
    "2"
);

/* link (ePDU1-LAB, SRV6-LAB, 'power chain', A5, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV6-LAB'),
    @asset_link_powerchain,
    "A5",
    "1"
);
/* link (ePDU2-LAB, SRV6-LAB, 'power chain', A5, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_device from t_bios_asset_device as t1 INNER JOIN t_bios_asset_element as t2 ON t1.id_asset_element = t2.id_asset_element where name = 'SRV6-LAB'),
    @asset_link_powerchain,
    "A5",
    "2"
);

SELECT @client_ui_properties := id_client FROM t_bios_client WHERE name = 'ui_properties';
UPDATE t_bios_client_info SET timestamp=UTC_TIMESTAMP(), ext='{"key1" : "value1", "key2" : "value2", "indicators_gauges" : { "power":{ "gauge_min" : 0, "gauge_max" : 50, "threshold_1" : 30, "threshold_2" : 40, "constant_tendency_threshold" : 1 }, "temperature":{ "gauge_min" : 10, "gauge_max" : 60, "threshold_1" : 36, "threshold_2" : 48, "constant_tendency_threshold" : 1 }, "humidity":{ "gauge_min" : 0, "gauge_max" : 100, "threshold_1" : 60, "threshold_2" : 80, "constant_tendency_threshold" : 1 }, "compute":{ "gauge_min" : 0, "gauge_max" : 100, "threshold_1" : 60, "threshold_2" : 80, "constant_tendency_threshold" : 1 }, "network":{ "gauge_min" : 0, "gauge_max" : 100, "threshold_1" : 60, "threshold_2" : 80, "constant_tendency_threshold" : 1 }, "storage":{ "gauge_min" : 0, "gauge_max" : 100, "threshold_1" : 60, "threshold_2" : 80, "constant_tendency_threshold" : 1 }, "space":{ "gauge_min" : 0, "gauge_max" : 420, "threshold_1" : 252, "threshold_2" : 336, "constant_tendency_threshold" : 1 } }, "rack_gauge_power": { "gauge_min": 0, "gauge_max": 5, "threshold_1": 3, "threshold_2": 4 }, "ups_gauge_power": { "gauge_min": 0, "gauge_max": 50, "threshold_1": 30, "threshold_2": 40 } } ' WHERE id_client=@client_ui_properties;


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

