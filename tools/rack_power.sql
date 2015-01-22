/* When modifying this data set, please keep in mind that you might need to modify REST API & DB rack_power tests as well */
USE box_utf8;

/* t_bios_asset_link_type*/

SELECT @id_link := id_asset_link_type FROM t_bios_asset_link_type WHERE name = 'power chain';

/* t_bios_asset_element_type */

SELECT @ae_rack := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='rack';
SELECT @ae_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';

/* t_bios_asset_device_type */

SELECT @device_ups := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @device_epdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'epdu';
SELECT @device_pdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'pdu';
SELECT @device_main := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'main';
SELECT @device_server := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'server';
SELECT @device_genset := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'genset';

/* t_bios_device_type */

SELECT @dvc_ups := id_device_type FROM t_bios_device_type WHERE name = 'ups';
SELECT @dvc_epdu := id_device_type FROM t_bios_device_type WHERE name = 'epdu';
SELECT @dvc_pdu := id_device_type FROM t_bios_device_type WHERE name = 'pdu';
SELECT @dvc_main := id_device_type FROM t_bios_device_type WHERE name = 'main';
SELECT @dvc_server := id_device_type FROM t_bios_device_type WHERE name = 'server';
SELECT @dvc_genset := id_device_type FROM t_bios_device_type WHERE name = 'genset';

/* Rack power #1 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8000, "RACK1", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8001, "epdu1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9001, 8001, @device_epdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10001, "epdu1_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10001, 8001);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8002, "ups1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9002, 8002, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10002, "ups1_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10002, 8002);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8003, "srv1_1", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9003, 8003, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10003, "srv1_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10003, 8003);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8004, "srv1_2", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9004, 8004, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10004, "srv1_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10004, 8004);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8005, "srv1_3", @ae_device, 8000); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9005, 8005, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10005, "srv1_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10005, 8005);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9001, 1, 9003, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9001, NULL, 9004, 2, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9002, NULL, 9001, NULL, @id_link);

/* Rack power #2 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8006, "RACK2", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8007, "epdu2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9007, 8007, @device_epdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10007, "epdu2_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10007, 8007);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8008, "ups2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9008, 8008, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10008, "ups2_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10008, 8008);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8009, "srv2_1", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9009, 8009, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10009, "srv2_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10009, 8009);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8010, "srv2_2", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9010, 8010, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10010, "srv2_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10010, 8010);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8011, "srv2_3", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9011, 8011, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10011, "srv2_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10011, 8011);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8012, "srv2_4", @ae_device, 8006); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9012, 8012, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10012, "srv2_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10012, 8012);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9007, 3, 9009, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9008, NULL, 9011, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9008, NULL, 9010, 4, @id_link);

/* Rack power #3 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8013, "RACK3", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8014, "pdu3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9014, 8014, @device_pdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10014, "pdu3_1_", @dvc_pdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10014, 8014);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8015, "ups3_1", @ae_device, 8013);
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9015, 8015, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10015, "ups3_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10015, 8015);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8016, "srv3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9016, 8016, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10016, "srv3_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10016, 8016);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8017, "srv3_2", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9017, 8017, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10017, "srv3_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10017, 8017);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8018, "srv3_3", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9018, 8018, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10018, "srv3_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10018, 8018);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8019, "srv3_4", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9019, 8019, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10019, "srv3_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10019, 8019);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8020, "ups3_2", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9020, 8020, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10020, "ups3_2_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10020, 8020);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8021, "ups3_3", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9021, 8021, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10021, "ups3_3_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10021, 8021);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8022, "epdu3_1", @ae_device, 8013); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9022, 8022, @device_epdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10022, "epdu3_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10022, 8022);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9014, 5, 9016, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9014, NULL, 9017, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9015, NULL, 9014, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9021, NULL, 9020, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9020, 7, 9019, 8, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9022, NULL, 9016, 6, @id_link);

/* Rack power #4 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8023, "RACK4", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8024, "pdu4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9024, 8024, @device_pdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10024, "pdu4_1_", @dvc_pdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10024, 8024);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8025, "epdu4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9025, 8025, @device_epdu);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10025, "epdu4_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10025, 8025);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8026, "ups4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9026, 8026, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10026, "ups4_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10026, 8026);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8027, "srv4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9027, 8027, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10027, "srv4_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10027, 8027);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8028, "srv4_2", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9028, 8028, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10028, "srv4_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10028, 8028);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8029, "srv4_3", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9029, 8029, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10029, "srv4_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10029, 8029);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8030, "srv4_4", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9030, 8030, @device_server);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10030, "srv4_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10030, 8030);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8031, "genset4_1", @ae_device, 8023); 
INSERT INTO t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) VALUES (9031, 8031, @device_genset);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9024, NULL, 9027, 11, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9024, NULL, 9028, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9025, 10, 9024, 12, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9026, 9, 9024, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 9031, NULL, 9029, NULL, @id_link);
