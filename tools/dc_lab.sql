use box_utf8;

delete from t_bios_asset_element        where id_asset_element>=10000 AND id_asset_element<11000;
delete from t_bios_asset_group_relation where id_asset_group_relation>=10000 AND id_asset_group_relation<11000;
delete from t_bios_asset_device         where id_asset_device>=10000 AND id_asset_device<11000;
delete from t_bios_asset_link           where id_link>=10000 AND id_link<11000;
delete from t_bios_asset_ext_attributes where id_asset_ext_attribute>=10000 AND id_asset_ext_attribute<11000;


insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10000, "DC-LAB",2,NULL);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10001, "ROOM1-LAB",3,10000);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10002, "RACK1-LAB",5,10001);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10003, "UPS1-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10004, "UPS2-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10005, "ePDU1-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10006, "ePDU2-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10007, "SRV1-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10008, "SRV2-LAB",6,10002);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10009, "MAIN-LAB",6,10000);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (10010, "GROUP1-LAB",1,10000);

insert into t_bios_asset_group_relation (id_asset_group_relation, id_asset_group, id_asset_element) values (10100, 10010,10009);

insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10200, 10003, 1 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10201, 10004, 1 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10202, 10005, 2 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10203, 10006, 2 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10204, 10007, 3 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10205, 10008, 3 );
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values (10206, 10009, 4 );

insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type) values (10300, 10206, 10200, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type) values (10301, 10206, 10201, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type) values (10302, 10200, 10202, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type) values (10303, 10201, 10203, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) values (10304, 10202, 10204, 1, 10, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) values (10305, 10203, 10204, 1, 10, 2);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) values (10306, 10202, 10205, 1, 11, 1);
insert into t_bios_asset_link (id_link, id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) values (10307, 10203, 10205, 1, 11, 2);


insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10500, "description","EATON Montobonnot Datacenter",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10501, "company","EATON",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10502, "site","Montbonnot",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10503, "country","France",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10504, "address","110 Rue Blaise Pascal, 38330 Montbonnot-Saint-Martin",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10505, "contact","john.smith@eaton.com",10000);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10506, "description","Lab Room",10001);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10507, "floor","0",10001);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10508, "description","BIOS Rack Validation",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10509, "free_space","27",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10510, "brand","EATON",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10511, "type","1",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10512, "height","27U",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10513, "width","600",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10514, "depth","800",10002);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10515, "description","UPS1 9PX 6kVA",10003);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10516, "description","UPS2 9PX 6kVA",10004);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10517, "description","ePDU1 Marlin",10005);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10518, "description","ePDU2 Marlin",10006);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10519, "description","SRV1 HP",10007);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10520, "location_u_height","2",10007);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10521, "location_u_pos","10",10007);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10522, "description","SRV2 HP",10008);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10523, "location_u_height","2",10007);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10524, "location_u_pos","10",10007);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10525, "description","MAIN 240V",10009);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10526, "feeds","2",10009);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10527, "description","input power chain",10010);
insert into t_bios_asset_ext_attributes ( id_asset_ext_attribute, keytag, value, id_asset_element) values (10528, "type","input_power",10010);


