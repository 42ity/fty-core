USE box_utf8;

/* t_bios_asset_element_type */

SELECT @ae_group := id_asset_element_type FROM t_bios_asset_element_type where name ='group';
SELECT @ae_datacenter := id_asset_element_type FROM t_bios_asset_element_type where name ='datacenter';
SELECT @ae_room := id_asset_element_type FROM t_bios_asset_element_type where name ='room';
SELECT @ae_row := id_asset_element_type FROM t_bios_asset_element_type where name ='row';
SELECT @ae_rack := id_asset_element_type FROM t_bios_asset_element_type where name ='rack';
SELECT @ae_device := id_asset_element_type FROM t_bios_asset_element_type where name ='device';

/* t_bios_device_type */

select @device_ups := id_asset_device_type from t_bios_asset_device_type where name = 'ups';
select @device_epdu := id_asset_device_type from t_bios_asset_device_type where name = 'epdu';
select @device_main := id_asset_device_type from t_bios_asset_device_type where name = 'main';
select @device_server := id_asset_device_type from t_bios_asset_device_type where name = 'server';
select @device_genset := id_asset_device_type from t_bios_asset_device_type where name = 'genset';

/* DC 1, is use for almost all tests for location topology */

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7000, "DC_LOC_01", @ae_datacenter, NULL); 

/* ROOMS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7001, "ROOM_LOC_01", @ae_room, 7000); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7002, "ROOM_LOC_02", @ae_room, 7000); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7003, "ROOM_LOC_50", @ae_room, NULL);  /* unlockated */

/* ROWS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7004, "ROW_LOC_01", @ae_row, 7001); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7005, "ROW_LOC_20", @ae_row, 7002); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7006, "ROW_LOC_21", @ae_row, 7002); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7007, "ROW_LOC_30", @ae_row, 7000); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7008, "ROW_LOC_50", @ae_row, NULL);    /* unlockated */ 

/* RACKS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7009, "RACK_LOC_010", @ae_rack, 7004); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7010, "RACK_LOC_011", @ae_rack, 7004); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7011, "RACK_LOC_20",  @ae_rack, 7005); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7012, "RACK_LOC_21",  @ae_rack, 7006); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7013, "RACK_LOC_30",  @ae_rack, 7000); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7014, "RACK_LOC_1",   @ae_rack, 7001); 
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7015, "RACK_LOC_50",  @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7016, "main_LOC_1",   @ae_device, 7000); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8000, 7016, @device_main);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7017, "genset_LOC_1", @ae_device, 7000); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8001, 7017, @device_genset);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7018, "ups_LOC_1",    @ae_device, 7000); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8002, 7018, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7019, "srv_LOC_40",   @ae_device, 7000); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8003, 7019, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7020, "srv_LOC_10",   @ae_device, 7009); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8004, 7020, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7021, "srv_LOC_11",   @ae_device, 7010); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8005, 7021, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7022, "ups_LOC_010",  @ae_device, 7009); 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8006, 7022, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7023, "srv_LOC_50",   @ae_device, NULL); /* unlockated */ 
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8007, 7023, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7024, "srv_LOC_51",   @ae_device, NULL); /* unlockated */
INSERT INTO t_bios_asset_device ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (8008, 7024, @device_server);
