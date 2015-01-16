/* When modifying this data set, please keep in mind that you might need to modify REST API & DB rack_power tests as well */
USE box_utf8;

/* t_bios_asset_link_type*/

SELECT @id_link := id_asset_link_type FROM t_bios_asset_link_type WHERE name = 'power chain';

/* t_bios_asset_element_type */

SELECT @ae_rack := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='rack';
SELECT @ae_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';

/* t_bios_device_type */

SELECT @device_ups := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @device_epdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'epdu';
SELECT @device_pdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'pdu';
SELECT @device_main := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'main';
SELECT @device_server := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'server';
SELECT @device_genset := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'genset';

/* Rack power #1 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8000, "RACK1",  @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8001, "epdu1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9001, 8001, @device_epdu);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8002, "ups1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9002, 8002, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8003, "srv1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9003, 8003, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8004, "srv1_2", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9004, 8004, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8005, "srv1_3", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9005, 8005, @device_server);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9001, 1, 9003, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9001, NULL, 9004, 2, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9002, NULL, 9001, NULL, @id_link);

/* Rack power #2 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8006, "RACK2",  @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8007, "epdu2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9007, 8007, @device_epdu);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8008, "ups2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9008, 8008, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8009, "srv2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9009, 8009, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8010, "srv2_2", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9010, 8010, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8011, "srv2_3", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9011, 8011, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8012, "srv2_3", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9012, 8012, @device_server);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9007, 3, 9009, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9008, NULL, 9011, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9008, NULL, 9010, 4, @id_link);

/* Rack power #3 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8013, "RACK3",  @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8014, "pdu3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9014, 8014, @device_pdu);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8015, "ups3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9015, 8015, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8016, "srv3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9016, 8016, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8017, "srv3_2", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9017, 8017, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8018, "srv3_3", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9018, 8018, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8019, "srv3_4", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9019, 8019, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8020, "ups3_2", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9020, 8020, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8021, "ups3_3", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9021, 8021, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8022, "epdu3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9022, 8022, @device_epdu);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9014, 5, 9016, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9014, NULL, 9017, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9015, NULL, 9014, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9021, NULL, 9020, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9020, 7, 9019, 8, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9022, NULL, 9016, 6, @id_link);

/* Rack power #4 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8023, "RACK4",  @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8024, "pdu4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9024, 8024, @device_pdu);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8025, "epdu4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9025, 8025, @device_epdu);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8026, "ups4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9026, 8026, @device_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8027, "srv4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9027, 8027, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8028, "srv4_2", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9028, 8028, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8029, "srv4_3", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9029, 8029, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8030, "srv4_4", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9030, 8030, @device_server);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8031, "genset4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (9031, 8031, @device_genset);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9024, NULL, 9027, 11, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9024, NULL, 9028, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9025, 10, 9024, 12, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9026, 9, 9024, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 9031, NULL, 9029, NULL, @id_link);

