/* When modifying this data set, please keep in mind that you might need to modify REST API & DB topology tests as well */
USE box_utf8;

/* t_bios_asset_element_type */

SELECT @ae_group := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='group';
SELECT @ae_datacenter := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='datacenter';
SELECT @ae_room := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='room';
SELECT @ae_row := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='row';
SELECT @ae_rack := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='rack';
SELECT @ae_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';

/* t_bios_device_type */

SELECT @device_ups := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @device_epdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'epdu';
SELECT @device_feed := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'feed';
SELECT @device_server := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'server';
SELECT @device_genset := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'genset';

/* ---------------- DC 01, is use for almost all tests for location topology  ------------------------------------------------------*/

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7000, "DC-LOC-01", @ae_datacenter, NULL);

/* ROOMS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7001, "ROOM-LOC-01", @ae_room, 7000);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7002, "ROOM-LOC-02", @ae_room, 7000);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7003, "ROOM-LOC-50", @ae_room, NULL);  /* unlockated */

/* ROWS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7004, "ROW-LOC-01", @ae_row, 7001);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7005, "ROW-LOC-20", @ae_row, 7002);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7006, "ROW-LOC-21", @ae_row, 7002);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7007, "ROW-LOC-30", @ae_row, 7000);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7008, "ROW-LOC-50", @ae_row, NULL);    /* unlockated */

/* RACKS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7009, "RACK-LOC-010", @ae_rack, 7004);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7010, "RACK-LOC-011", @ae_rack, 7004);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7011, "RACK-LOC-20",  @ae_rack, 7005);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7012, "RACK-LOC-21",  @ae_rack, 7006);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7013, "RACK-LOC-30",  @ae_rack, 7000);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7014, "RACK-LOC-1",   @ae_rack, 7001);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7015, "RACK-LOC-50",  @ae_rack, NULL); /* unlockated */

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7016, "feed-LOC-1", @ae_device, @device_feed, 7000);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7017, "genset-LOC-1", @ae_device, @device_genset, 7000);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7018, "ups-LOC-1", @ae_device, @device_ups, 7000);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7019, "srv-LOC-40", @ae_device, @device_server, 7000);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7020, "srv-LOC-10", @ae_device, @device_server, 7009);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7021, "srv-LOC-11", @ae_device, @device_server, 7010);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7022, "ups-LOC-010", @ae_device, @device_ups, 7009);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7023, "srv-LOC-50", @ae_device, @device_server, NULL); /* unlockated */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7024, "srv-LOC-51", @ae_device, @device_server, NULL); /* unlockated */

/* GROUPS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7025, "inputpowergroup DC-LOC-01", @ae_group, 7000);
INSERT INTO t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) VALUES (9000, "type", "N_A", 7025);

INSERT INTO t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
VALUES
(
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = "inputpowergroup DC-LOC-01"),
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'feed-LOC-1')
);

INSERT INTO t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
VALUES
(
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'inputpowergroup DC-LOC-01'),
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'genset-LOC-1')
);

INSERT INTO t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
VALUES
(
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'inputpowergroup DC-LOC-01'),
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'ups-LOC-1')
);

/* ----------------------------------------------DC 02 -----------------------------------------------------------------*/

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7026, "DC-LOC-02", @ae_datacenter, NULL);

/* RACKS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7027, "RACK-LOC-6", @ae_rack, 7026);

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (7028, "DEVICE1", @ae_device, @device_feed, 7026);

/* -------------------------------------------- DC 03------------------------------------------------------------------------- */

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7029, "DC-LOC-03", @ae_datacenter, NULL);

/* ROOMS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7030, "ROOM-LOC-6", @ae_room, 7029);

/* ROWS*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7031, "ROW-LOC-6",  @ae_row, 7029);

/* ---------------------------------------------- DC 04, empty ----------------------------------------------------------------*/

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (7032, "DC-LOC-04", @ae_datacenter, NULL);
"""
