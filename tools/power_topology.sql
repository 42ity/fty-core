use box_utf8;
insert into t_bios_asset_device_type (id_asset_device_type , name) values (5, "mgenset");



insert into t_bios_asset_device_type (id_asset_device_type , name) values (6, "sink");

insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5000, "serv2", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5001, "ePDUA", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5002, "ePDUB", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5003, "Upps1", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5004, "main2", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5005, "GenSetA", 6 , NULL); 

insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6000, 5000, 3);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6001, 5001, 2);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6002, 5002, 2);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6003, 5003, 1);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6004, 5004, 4);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6005, 5005, 5);

insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6001 , 2,6000 , 1, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6002 , 4,6000 , 3, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6000 , 5, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6001 , 7, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6002 , 6, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6004 , 8,6003 , NULL, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6005 , 9,6003 , NULL, 1);

insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4999, "power_inputgrp_test", 1, NULL);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5005);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5003);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5002);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5000);



insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4998, "power_inputgrp_empty", 1, NULL);



/* prepared to insert */

INSERT INTO t_bios_device_type (name) VALUES ("sink");

SELECT @id_ups := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @id_sink := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'sink';
SELECT @id_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';
SELECT @id_link := id_asset_link_type FROM t_bios_asset_link_type WHERE name = 'power chain';

/* power topology from: #1 */
/* no need to insert anything */

/* power topology from: #2 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5020, "UPSFROM2", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6020, 5020, @id_ups);


/* power topology from: #3 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5021, "UPSFROM3", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6021, 5021, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5022, "SINK1", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6022, 5022, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5023, "SINK2", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6023, 5023, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6022, NULL, 6021 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6023, NULL, 6021 , NULL, @id_link);

/* power topology from: #4 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5024, "UPSFROM4", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6024, 5024, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5025, "SINK3", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6025, 5025, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5026, "SINK4", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6026, 5026, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5027, "SINK5", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6027, 5027, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6025, NULL, 6024 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6026, NULL, 6024 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6024, 1, 6027 , 2, @id_link);

/* power topology from: #5 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5028, "UPSFROM5", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6028, 5028, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5029, "SINK6", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6029, 5029, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5030, "SINK7", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6030, 5030, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5031, "SINK8", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6031, 5031, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5032, "SINK9", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6032, 5032, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5033, "SINK10", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6033, 5033, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6029, NULL, 6028 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6030, NULL, 6028 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6028, NULL, 6031 , 3, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6028, NULL, 6032 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6028, 4, 6033 , NULL, @id_link);

/* power topology from: #6 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5034, "UPSFROM6", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6034, 5034, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5035, "SINK11", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6035, 5035, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5036, "SINK12", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6036, 5036, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5037, "SINK13", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6037, 5037, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6034, NULL, 6035 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6034, NULL, 6036 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6034, NULL, 6037 , NULL, @id_link);

/* power topology from: #7 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5038, "UPSFROM7", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6038, 5038, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5039, "SINK14", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6039, 5039, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6038, NULL, 6039 , NULL, @id_link);

/* power topology from: #8 */
/* no need to insert anything */

/* power topology from: #9 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5040, "UPSFROM9", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6040, 5040, @id_ups);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6040, 5, 6040 , 6, @id_link);

/* power topology from: #10 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5041, "UPSFROM10", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6041, 5041, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5042, "SINK15", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6042, 5042, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6041, NULL, 6042 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6041, NULL, 6042 , NULL, @id_link);

/* power topology from: #11 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5043, "UPSFROM11", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6043, 5043, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5044, "SINK16", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6044, 5044, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6043, NULL, 6044 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6044, NULL, 6043 , NULL, @id_link);

/* power topology to: #1 */
/* no need to insert anything */

/* power topology to: #2 */
/* no need to insert anything */

/* power topology to: #3 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5045, "UPSTO3", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6045, 5045, @id_ups);

/* power topology to: #4 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5046, "UPSTO4", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6046, 5046, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5047, "SINK36", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6047, 5047, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5048, "SINK37", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6048, 5048, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6046, NULL, 6047 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6046, NULL, 6048 , NULL, @id_link);

/* power topology to: #5 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5049, "UPSTO5", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6049, 5049, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5050, "SINK38", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6050, 5050, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5051, "SINK39", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6051, 5051, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5052, "SINK17", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6052, 5052, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5053, "SINK18", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6053, 5053, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6049, NULL, 6050 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6049, NULL, 6051 , NULL, @id_link);


INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6052, NULL, 6049 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6053, NULL, 6049 , NULL, @id_link);

/* power topology to: #6 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5054, "UPSTO6", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6054, 5054, @id_ups);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5055, "SINK19", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6055, 5055, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5056, "SINK20", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6056, 5056, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5057, "SINK21", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6057, 5057, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5058, "SINK22", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6058, 5058, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5059, "SINK23", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6059, 5059, @id_sink);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5060, "SINK40", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6060, 5060, @id_sink);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6055, 1, 6057 , 2, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6056, NULL, 6057 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6057, 3, 6058 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6057, NULL, 6054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6057, NULL, 6059 , 4, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6058, NULL, 6054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6059, NULL, 6054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6054, NULL, 6060 , NULL, @id_link);

/* power topology to: #7 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent )              VALUES (5061, "UPSTO7", @id_device , NULL); 
INSERT INTO t_bios_asset_device  ( id_asset_device, id_asset_element, id_asset_device_type ) VALUES (6061, 5061, @id_ups);


INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 6061, 5, 6061 , 6, @id_link);
