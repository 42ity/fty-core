/* When modifying this data set, please keep in mind that you might need to modify REST API & DB power tests as well */
use box_utf8;

insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5000, "serv2",   6 , 3, NULL);
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5001, "ePDUA",   6 , 2, NULL);
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5002, "ePDUB",   6 , 2, NULL);
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5003, "Upps1",   6 , 1, NULL);
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5004, "feed2",   6 , 4, NULL);
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_subtype, id_parent ) values (5005, "GenSetA", 6 , 5, NULL);

insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5001 , 2,5000 , 1, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5002 , 4,5000 , 3, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5003 , NULL,5000 , 5, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5003 , NULL,5001 , 7, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5003 , NULL,5002 , 6, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5004 , 8,5003 , NULL, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 5005 , 9,5003 , NULL, 1);

insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4999, "power_inputgrp_test", 1, NULL);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5005);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5003);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5002);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5000);

/* prepared to insert */

INSERT INTO t_bios_asset_device_type (name) VALUES ("sink");

SELECT @id_ups := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @id_sink := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'sink';
SELECT @id_epdu := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'epdu';
SELECT @id_feed := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'feed';
SELECT @id_server := id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'server';

SELECT @id_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';
SELECT @id_link := id_asset_link_type FROM t_bios_asset_link_type WHERE name = 'power chain';


/* power topology from: #1 */
/* no need to insert anything */

/* power topology from: #2 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5020, "UPSFROM2", @id_device, @id_ups, NULL);


/* power topology from: #3 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5021, "UPSFROM3", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5022, "SINK1", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5023, "SINK2", @id_device , @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5022, NULL, 5021 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5023, NULL, 5021 , NULL, @id_link);

/* power topology from: #4 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5024, "UPSFROM4", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5025, "SINK3", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5026, "SINK4", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5027, "SINK5", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5025, NULL, 5024 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5026, NULL, 5024 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5024, "ABCD", 5027 , 2, @id_link);

/* power topology from: #5 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5028, "UPSFROM5", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5029, "SINK6", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5030, "SINK7", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5031, "SINK8", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5032, "SINK9", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5033, "SINK10", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5029, NULL, 5028 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5030, NULL, 5028 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5028, NULL, 5031 , 3, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5028, NULL, 5032 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5028, 4, 5033 , NULL, @id_link);

/* power topology from: #6 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5034, "UPSFROM6", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5035, "SINK11", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5036, "SINK12", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5037, "SINK13", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5034, NULL, 5035 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5034, NULL, 5036 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5034, NULL, 5037 , NULL, @id_link);

/* power topology from: #7 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5038, "UPSFROM7", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5039, "SINK14", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5038, NULL, 5039 , NULL, @id_link);

/* power topology from: #8 */
/* no need to insert anything */

/* power topology from: #9 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5040, "UPSFROM9", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5040, 5, 5040 , 6, @id_link);

/* power topology from: #10 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5041, "UPSFROM10", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5042, "SINK15", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5041, NULL, 5042 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5041, 5, 5042 , NULL, @id_link);

/* power topology from: #11 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5043, "UPSFROM11", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5044, "SINK16", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5043, NULL, 5044 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5044, NULL, 5043 , NULL, @id_link);

/* power topology to: #1 */
/* no need to insert anything */

/* power topology to: #2 */
/* no need to insert anything */

/* power topology to: #3 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5045, "UPSTO3", @id_device, @id_ups, NULL);

/* power topology to: #4 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5046, "UPSTO4", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5047, "SINK36", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5048, "SINK37", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5046, NULL, 5047 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5046, NULL, 5048 , NULL, @id_link);

/* power topology to: #5 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5049, "UPSTO5", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5050, "SINK38", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5051, "SINK39", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5052, "SINK17", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5053, "SINK18", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5049, NULL, 5050 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5049, NULL, 5051 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5052, NULL, 5049 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5053, NULL, 5049 , NULL, @id_link);

/* power topology to: #6 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5054, "UPSTO6", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5055, "SINK19", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5056, "SINK20", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5057, "SINK21", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5058, "SINK22", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5059, "SINK23", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5060, "SINK40", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5055, 1, 5057 , 2, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5056, NULL, 5057 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5057, 3, 5058 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5057, NULL, 5054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5057, NULL, 5059 , 4, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5058, NULL, 5054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5059, NULL, 5054 , NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5054, NULL, 5060 , NULL, @id_link);

/* power topology to: #7 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5061, "UPSTO7", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5061, 5, 5061 , 6, @id_link);

/* power topology to: #8 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5062, "UPSTO8", @id_device, @id_ups, NULL);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5063, "SINK24", @id_device, @id_sink, NULL);
INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5064, "SINK25", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5064, NULL, 5062, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5063, NULL, 5064, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5062, NULL, 5063, NULL, @id_link);

/* power topology to: #9 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5065, "UPSTO9", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5066, "SINK26", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5067, "SINK27", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5068, "SINK28", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5069, "SINK29", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5070, "SINK30", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5071, "SINK31", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5072, "SINK32", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5073, "SINK33", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5072, NULL, 5065, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5073, NULL, 5065, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5070, NULL, 5072, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5066, NULL, 5070, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5067, NULL, 5070, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5071, NULL, 5073, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5069, NULL, 5071, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5068, NULL, 5071, NULL, @id_link);

/* power topology to: #10 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5074, "UPSTO10", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5075, "SINK34", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5075, NULL, 5074, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5075, 5, 5074, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5075, NULL, 5074, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5075, 5, 5074, NULL, @id_link);

/* power topology to: #11 */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5076, "UPSTO11", @id_device, @id_ups, NULL);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent )              VALUES (5077, "SINK35", @id_device, @id_sink, NULL);

INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5077, NULL, 5076, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5076, NULL, 5077, NULL, @id_link);

/* power topology group/datacenter #1, #2*/

SELECT @ae_group := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='group';
SELECT @ae_datacenter := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='datacenter';
SELECT @ae_room := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='room';
SELECT @ae_row := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='row';
SELECT @ae_rack := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='rack';
SELECT @ae_device := id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device';


SELECT @asset_link_powerchain := id_asset_link_type FROM t_bios_asset_link_type WHERE name = 'power chain';

/* DC */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (5078, "DC-05", @ae_datacenter,  NULL);

/* ROOMS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (5079, "ROOM1-05", @ae_room, 5078);

/* RACKS */

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_parent) VALUES (5080, "RACK1-05", @ae_rack, 5079);

/* DEVICES*/

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5081, "UPS1-05", @ae_device, @id_ups, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5082, "UPS2-05", @ae_device, @id_ups, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5083, "ePDU1-05", @ae_device, @id_epdu, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5084, "ePDU2-05", @ae_device, @id_epdu, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5085, "SRV1-05",  @ae_device, @id_server, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5086, "SRV2-05", @ae_device, @id_server, 5080);

INSERT INTO t_bios_asset_element ( id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5087, "MAIN-05", @ae_device, @id_feed, 5078);

/* GROUPS */
INSERT INTO t_bios_asset_element ( id_asset_element, name , id_type, id_parent) VALUES (5088, "GROUP1-05", @ae_group, 5078);
INSERT INTO t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) VALUES (9001, "type", "input power", 5078);

INSERT INTO t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
VALUES
(
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'GROUP1-05'),
    (SELECT id_asset_element FROM `t_bios_asset_element` WHERE name = 'MAIN-05')
);

/* Asset links */
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5087, NULL, 5081, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5087, NULL, 5082, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5081, NULL, 5083, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5082, NULL, 5084, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5083, NULL, 5085, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5083, NULL, 5086, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5084, NULL, 5085, NULL, @id_link);
INSERT INTO t_bios_asset_link (id_link, id_asset_device_src, src_out, id_asset_device_dest, dest_in, id_asset_link_type ) VALUES (NULL, 5084, NULL, 5086, NULL, @id_link);

/* New group */
insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4998, "power_inputgrp_empty", 1, NULL);
insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (5089, "POT", 1, NULL);

/* New group relation */
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5086);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5087);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5081);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5083);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5084);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5085);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 5089, 5075);
