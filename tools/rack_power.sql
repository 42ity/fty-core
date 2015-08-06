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
SELECT @dvc_main := id_device_type FROM t_bios_device_type WHERE name = 'main';
SELECT @dvc_server := id_device_type FROM t_bios_device_type WHERE name = 'server';
SELECT @dvc_genset := id_device_type FROM t_bios_device_type WHERE name = 'genset';

/* Rack power #1 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8000, "RACK1", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8001, "epdu1_1", @ae_device, 8000, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10001, "epdu1_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10001, 8001);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8002, "ups1_1", @ae_device, 8000, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10002, "ups1_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10002, 8002);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8003, "srv1_1", @ae_device, 8000, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10003, "srv1_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10003, 8003);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8004, "srv1_2", @ae_device, 8000, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10004, "srv1_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10004, 8004);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8005, "srv1_3", @ae_device, 8000, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10005, "srv1_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10005, 8005);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8001, 1, 8003, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8001, NULL, 8004, 2, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8002, NULL, 8001, NULL, @id_link);

/* Rack power #2 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (8006, "RACK2", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8007, "epdu2_1", @ae_device, 8006, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10007, "epdu2_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10007, 8007);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8008, "ups2_1", @ae_device, 8006, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10008, "ups2_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10008, 8008);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8009, "srv2_1", @ae_device, 8006, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10009, "srv2_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10009, 8009);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8010, "srv2_2", @ae_device, 8006, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10010, "srv2_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10010, 8010);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8011, "srv2_3", @ae_device, 8006, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10011, "srv2_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10011, 8011);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8012, "srv2_4", @ae_device, 8006, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10012, "srv2_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10012, 8012);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8007, 3, 8009, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8008, NULL, 8011, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8008, NULL, 8010, 4, @id_link);

/* Rack power #3 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8013, "RACK3", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8014, "pdu3_1", @ae_device, 8013, @device_pdu); 

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8015, "ups3_1", @ae_device, 8013, @device_ups);
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10015, "ups3_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10015, 8015);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8016, "srv3_1", @ae_device, 8013, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10016, "srv3_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10016, 8016);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8017, "srv3_2", @ae_device, 8013, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10017, "srv3_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10017, 8017);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8018, "srv3_3", @ae_device, 8013, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10018, "srv3_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10018, 8018);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8019, "srv3_4", @ae_device, 8013, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10019, "srv3_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10019, 8019);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8020, "ups3_2", @ae_device, 8013, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10020, "ups3_2_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10020, 8020);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8021, "ups3_3", @ae_device, 8013, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10021, "ups3_3_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10021, 8021);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8022, "epdu3_1", @ae_device, 8013, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10022, "epdu3_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10022, 8022);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8014, 5, 8016, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8014, NULL, 8017, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8015, NULL, 8014, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8021, NULL, 8020, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8020, 7, 8019, 8, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8022, NULL, 8016, 6, @id_link);

/* Rack power #4 */

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8023, "RACK4", @ae_rack, NULL); /* unlockated */ 

/* DEVICES*/

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8024, "pdu4_1", @ae_device, 8023, @device_pdu); 

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8025, "epdu4_1", @ae_device, 8023, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10025, "epdu4_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10025, 8025);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8026, "ups4_1", @ae_device, 8023, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10026, "ups4_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10026, 8026);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8027, "srv4_1", @ae_device, 8023, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10027, "srv4_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10027, 8027);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8028, "srv4_2", @ae_device, 8023, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10028, "srv4_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10028, 8028);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8029, "srv4_3", @ae_device, 8023, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10029, "srv4_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10029, 8029);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8030, "srv4_4", @ae_device, 8023, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10030, "srv4_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_ma_relation, id_discovered_device, id_asset_element) VALUES (NULL, 10030, 8030);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8031, "genset4_1", @ae_device, 8023, @device_genset); 

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8024, NULL, 8027, 11, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8024, NULL, 8028, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8025, 10, 8024, 12, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8026, 9, 8024, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (NULL, 8031, NULL, 8029, NULL, @id_link);

/* Rack power #101  */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8101, "RACK101", @ae_rack, NULL);

/* DEVICES*/
/* epdu101_1 */
/*   */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8102, "epdu101_1", @ae_device, 8101, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10102, "epdu101_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10102, 8102);

/* epdu101_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8103, "epdu101_2", @ae_device, 8101, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10103, "epdu101_2_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10103, 8103);

/* srv101_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8104, "srv101_1", @ae_device, 8101, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10104, "srv101_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10104, 8104);

/* srv101_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8105, "srv101_2", @ae_device, 8101, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10105, "srv101_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10105, 8105);

/* srv101_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8106, "srv101_3", @ae_device, 8101, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10106, "srv101_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10106, 8106);

/* srv101_4 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8107, "srv101_4", @ae_device, 8101, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10107, "srv101_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10107, 8107);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8102, NULL, 8104, 14, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8102, NULL, 8105, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8102, NULL, 8106, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8103, NULL, 8106, 15, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8102, 16, 8107, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8103, NULL, 8107, 13, @id_link);

/* Rack power #102  */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8108, "RACK102", @ae_rack, NULL);

/* DEVICES*/
/* ups102_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8109, "ups102_1", @ae_device, 8108, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10109, "ups102_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10109, 8109);

/* epdu102_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8110, "epdu102_1", @ae_device, 8108, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10110, "epdu102_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10110, 8110);

/* epdu102_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8111, "epdu102_2", @ae_device, 8108, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10111, "epdu102_2_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10111, 8111);

/* srv102_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8112, "srv102_1", @ae_device, 8108, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10112, "srv102_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10112, 8112);

/* srv102_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8113, "srv102_2", @ae_device, 8108, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10113, "srv102_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10113, 8113);

/* srv102_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8114, "srv102_3", @ae_device, 8108, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10114, "srv102_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10114, 8114);

/* srv102_4 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8115, "srv102_4", @ae_device, 8108, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10115, "srv102_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10115, 8115);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8109, NULL, 8110, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8109, NULL, 8111, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8110, 17, 8112, 18, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8110, NULL, 8113, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8110, NULL, 8114, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8110, NULL, 8115, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8111, NULL, 8114, 19, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8111, 20, 8115, NULL, @id_link);

/* Rack power #103 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8116, "RACK103", @ae_rack, NULL);

/* DEVICES*/
/* main103_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8117, "main103_1", @ae_device, NULL, @device_main); 

/* ups103_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8118, "ups103_1", @ae_device, 8116, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10118, "ups103_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10118, 8118);

/* ups103_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8119, "ups103_2", @ae_device, 8116, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10119, "ups103_2_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10119, 8119);

/* pdu103_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8120, "pdu103_1", @ae_device, 8116, @device_pdu); 

/* pdu103_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8121, "pdu103_2", @ae_device, 8116, @device_pdu); 

/* srv103_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8122, "srv103_1", @ae_device, 8116, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10122, "srv103_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10122, 8122);

/* srv103_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8123, "srv103_2", @ae_device, 8116, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10123, "srv103_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10123, 8123);

/* srv103_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8124, "srv103_3", @ae_device, 8116, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10124, "srv103_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10124, 8124);

/* srv103_4 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8125, "srv103_4", @ae_device, 8116, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10125, "srv103_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10125, 8125);


INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8117, NULL, 8118, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8117, 21, 8119, 25, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8118, 27, 8120, 26, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8119, NULL, 8121, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8120, NULL, 8122, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8120, NULL, 8123, 22, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8120, NULL, 8124, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8120, NULL, 8125, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8121, NULL, 8124, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8121, 23, 8125, 24, @id_link);

/* Rack power #104 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8126, "RACK104", @ae_rack, NULL);

/* DEVICES*/
/* main104_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8127, "main104_1", @ae_device, NULL, @device_main); 

/* pdu104_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8128, "pdu104_1", @ae_device, 8126, @device_pdu); 

/* pdu104_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8129, "pdu104_2", @ae_device, 8126, @device_pdu); 

/* srv104_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8130, "srv104_1", @ae_device, 8126, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10130, "srv104_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10130, 8130);

/* srv104_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8131, "srv104_2", @ae_device, 8126, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10131, "srv104_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10131, 8131);

/* srv104_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8132, "srv104_3", @ae_device, 8126, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10132, "srv104_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10132, 8132);

/* srv104_4 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8133, "srv104_4", @ae_device, 8126, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10133, "srv104_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10133, 8133);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, NULL, 8128, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, NULL, 8129, 30, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, NULL, 8130, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, 29, 8131, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, 28, 8132, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8127, NULL, 8133, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8129, NULL, 8132, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8129, NULL, 8133, NULL, @id_link);

/* Rack power #105 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8134, "RACK105", @ae_rack, NULL);

/* DEVICES*/
/* main105_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8135, "main105_1", @ae_device, NULL, @device_main); 

/* pdu105_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8136, "pdu105_1", @ae_device, NULL, @device_pdu); 

/* epdu105_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8137, "epdu105_1", @ae_device, 8134, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10137, "epdu105_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10137, 8137);

/* srv105_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8138, "srv105_1", @ae_device, 8134, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10138, "srv105_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10138, 8138);

/* srv105_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8139, "srv105_2", @ae_device, 8134, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10139, "srv105_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10139, 8139);

/* srv105_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8140, "srv105_3", @ae_device, 8134, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10140, "srv105_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10140, 8140);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8135, 34, 8136, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8136, NULL, 8137, 35, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8136, NULL, 8138, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8136, 32, 8139, 33, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8136, 38, 8140, 31, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8137, NULL, 8138, 36, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8137, NULL, 8139, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8137, 37, 8140, NULL, @id_link);

/* Rack power #106 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8141, "RACK106", @ae_rack, NULL);

/* DEVICES*/
/* main106_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8142, "main106_1", @ae_device, NULL, @device_main); 

/* pdu106_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8143, "pdu106_1", @ae_device, NULL, @device_pdu); 

/* ups106_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8144, "ups106_1", @ae_device, 8141, @device_ups); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10144, "ups106_1_", @dvc_ups);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10144, 8144);

/* pdu106_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8145, "pdu106_2", @ae_device, 8141, @device_pdu); 

/* srv106_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8146, "srv106_1", @ae_device, 8141, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10146, "srv106_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10146, 8146);

/* srv106_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8147, "srv106_2", @ae_device, 8141, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10147, "srv106_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10147, 8147);

/* srv106_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8148, "srv106_3", @ae_device, 8141, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10148, "srv106_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10148, 8148);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8142, 39, 8143, 40, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8143, 41, 8144, 42, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8143, 43, 8146, 44, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8143, 45, 8147, 46, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8143, 48, 8148, 47, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8144, NULL, 8145, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8145, NULL, 8146, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8145, NULL, 8147, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8145, NULL, 8148, NULL, @id_link);

/* Rack power #107 + #108 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8149, "RACK107", @ae_rack, NULL);
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent) VALUES (8150, "RACK108", @ae_rack, NULL);

/* DEVICES*/
/* main107_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8151, "main107_1", @ae_device, NULL, @device_main); 

/* epdu107_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8152, "epdu107_1", @ae_device, 8149, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10152, "epdu107_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10152, 8152);

/* epdu107_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8153, "epdu107_2", @ae_device, 8149, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10153, "epdu107_2_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10153, 8153);

/* epdu108_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8154, "epdu108_1", @ae_device, 8150, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10154, "epdu108_1_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10154, 8154);

/* epdu108_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8155, "epdu108_2", @ae_device, 8150, @device_epdu); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10155, "epdu108_2_", @dvc_epdu);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10155, 8155);

/* srv107_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8156, "srv107_1", @ae_device, 8149, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10156, "srv107_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10156, 8156);

/* srv107_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8157, "srv107_2", @ae_device, 8149, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10157, "srv107_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10157, 8157);

/* srv107_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8158, "srv107_3", @ae_device, 8149, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10158, "srv107_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10158, 8158);

/* srv107_4 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8159, "srv107_4", @ae_device, 8149, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10159, "srv107_4_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10159, 8159);

/* srv108_1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8160, "srv108_1", @ae_device, 8150, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10160, "srv108_1_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10160, 8160);

/* srv108_2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8161, "srv108_2", @ae_device, 8150, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10161, "srv108_2_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10161, 8161);

/* srv108_3 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent, id_subtype) VALUES (8162, "srv108_3", @ae_device, 8150, @device_server); 
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES (10162, "srv108_3_", @dvc_server);
INSERT INTO t_bios_monitor_asset_relation  (id_discovered_device, id_asset_element) VALUES (10162, 8162);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8151, 50, 8152, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8151, NULL, 8153, 51, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8151, NULL, 8154, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8151, 52, 8155, 53, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8152, 56, 8156, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8152, NULL, 8157, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8152, NULL, 8158, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8153, NULL, 8156, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8153, NULL, 8157, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8153, NULL, 8158, 49, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8154, NULL, 8160, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8154, NULL, 8161, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8154, NULL, 8162, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8154, 55, 8159, NULL, @id_link);

INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8155, NULL, 8160, 57, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8155, NULL, 8161, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8155, 58, 8162, 59, @id_link);
INSERT INTO t_bios_asset_link (id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type) VALUES (8155, 60, 8159, NULL, @id_link);

