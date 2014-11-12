use box_utf8;

insert into t_bios_asset_link_type (id_asset_link_type, name) values (NULL, "power chain");

insert into t_bios_device_type (id_device_type, name) values (1, "not_classified");
insert into t_bios_client (id_client, name) values (1, "nmap");

insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);
insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);

insert into t_bios_client(id_client,name) values(NULL,"mymodule");
insert into t_bios_client(id_client,name) values(NULL,"admin");
insert into t_bios_client(id_client,name) values(NULL,"NUT");

insert into t_bios_device_type(id_device_type,name) values (NULL,"ups");
insert into t_bios_device_type(id_device_type,name) values (NULL,"epdu");
insert into t_bios_device_type(id_device_type,name) values (NULL,"serv");

insert into t_bios_asset_device_type (id_asset_device_type, name ) values (NULL,"ups");
insert into t_bios_asset_device_type (id_asset_device_type, name ) values (NULL,"epdu");
insert into t_bios_asset_device_type (id_asset_device_type, name ) values (NULL,"serv");
insert into t_bios_asset_device_type (id_asset_device_type, name ) values (NULL,"main");

insert into t_bios_asset_element_type (id_asset_element_type, name) values (1, "group");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (2, "datacenter");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (3, "room");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (4, "row");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (5, "rack");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (6, "device");

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "DC1",2,NULL);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM1",3,1);

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "ROW1",4,2);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "RACK1",5,3);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "serv1",6,4);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type, mac) values (NULL, 5, 3, conv("112233445566",16,10));
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "epdu",6,2);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type)   values (NULL, 6,2);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "ups",6,2);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type)   values (NULL, 7,1);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent)  values (NULL, "main",6,1);
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type)   values (NULL, 8,4);

insert into t_bios_asset_element (id_asset_element, name , id_type,id_parent)     values (NULL, "group1",1,NULL);
insert into t_bios_asset_group_relation (id_asset_group_relation, id_asset_group, id_asset_element) values (NULL, 1,6);
insert into t_bios_asset_group_relation (id_asset_group_relation, id_asset_group, id_asset_element) values (NULL, 1,7);
insert into t_bios_asset_group_relation (id_asset_group_relation, id_asset_group, id_asset_element) values (NULL, 1,8);


insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest,  id_asset_link_type, src_out, dest_in) values (NULL, 4,3,1,1,2);
insert into t_bios_asset_link (id_link,id_asset_device_src,id_asset_device_dest, id_asset_link_type) values (NULL, 3,2,1);
insert into t_bios_asset_link (id_link,id_asset_device_src,id_asset_device_dest, id_asset_link_type) values (NULL, 2,1,1);

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "DC2",2, NULL);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM2",3,10);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM3",3,10);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM4",3,NULL);

insert into t_bios_asset_ext_attributes (id_asset_ext_attribute, keytag, value, id_asset_element) values (NULL, "total_facility_load","2",10);
insert into t_bios_asset_ext_attributes (id_asset_ext_attribute, keytag, value, id_asset_element) values (NULL, "contact","mike@nn.com",10);

insert into t_bios_asset_ext_attributes (id_asset_ext_attribute, keytag, value, id_asset_element) values (NULL, "description","room is very cute",13);
insert into t_bios_asset_ext_attributes (id_asset_ext_attribute, keytag, value, id_asset_element) values (NULL, "flowers","yes, in this room there are only flowers",13);

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW4",4, NULL);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW2",4, 11);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW3",4, 12);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW5",4, 12);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW6",4, 10);

insert into t_bios_discovered_device(id_discovered_device, id_device_type , name) values (NULL, 1, "measures");
insert into t_bios_measure_key (id_key, keytag) values (NULL, "key-tempetarure");
insert into t_bios_measure_key (id_key, keytag) values (NULL, "key-width");
insert into t_bios_measure_subkey (id_subkey , subkeytag, scale ) values (NULL, "top-left", 1);
insert into t_bios_measure_subkey (id_subkey , subkeytag, scale ) values (NULL, "bottom-center", -2);
insert into t_bios_client_info_measurements(id_measurements , id_key , id_subkey , value , timestamp , id_client, id_discovered_device ) values ( NULL, 1,1,3, "2014-11-12 09:45:59", 1, 1);
insert into t_bios_client_info_measurements(id_measurements , id_key , id_subkey , value , timestamp , id_client, id_discovered_device ) values ( NULL, 1,1,32, "2014-11-12 09:46:59", 1, 1);
insert into t_bios_client_info_measurements(id_measurements , id_key , id_subkey , value , timestamp , id_client, id_discovered_device ) values ( NULL, 2,1,31, "2014-11-12 09:47:59", 1, 1);
insert into t_bios_client_info_measurements(id_measurements , id_key , id_subkey , value , timestamp , id_client, id_discovered_device ) values ( NULL, 2,2,12, "2014-11-12 09:48:59", 1, 1);
insert into t_bios_client_info_measurements(id_measurements , id_key , id_subkey , value , timestamp , id_client, id_discovered_device ) values ( NULL, 1,2,142,"2014-11-12 09:49:59", 1, 1);

INSERT INTO t_bios_net_history (id_net_history, command, ip, mask, mac, name, timestamp) VALUES (7,  "a", "fe80", 64, "wlo1", "c4:d9:87:2f:dc:7b", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (id_net_history, command, ip, mask, mac, name, timestamp) VALUES (8,  "m", "192.168.1.0", 24, "", "", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (id_net_history, command, ip, mask, mac, name, timestamp) VALUES (9,  "a", "10.231.107.0", 24, "enp0s25", "a0:1d:48:b7:e2:4e", UTC_TIMESTAMP());
INSERT INTO t_bios_net_history (id_net_history, command, ip, mask, mac, name, timestamp) VALUES (10, "d", "10.0.0.0", 8, "", "", UTC_TIMESTAMP());
