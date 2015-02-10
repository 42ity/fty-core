use box_utf8;

select @asset_element_datacenter := id_asset_element_type from t_bios_asset_element_type where name ='datacenter';
select @asset_element_room := id_asset_element_type from t_bios_asset_element_type where name ='room';
select @asset_element_rack := id_asset_element_type from t_bios_asset_element_type where name ='rack';
select @asset_element_device := id_asset_element_type from t_bios_asset_element_type where name ='device';

select @device_ups := id_device_type from t_bios_device_type where name = 'ups';
select @device_server := id_device_type from t_bios_device_type where name = 'server';

select @asset_device_ups := id_asset_device_type from t_bios_asset_device_type where name = 'ups';
select @asset_device_server := id_asset_device_type from t_bios_asset_device_type where name = 'server';

select @asset_link_powerchain := id_asset_link_type from t_bios_asset_link_type where name = 'power chain';

/* Datacenter */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477000", "DC-US477", @asset_element_datacenter,  NULL);

/* Room */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477001", "ROOM1-US477", @asset_element_room, "477000");

/* RACKS */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477002", "RACK1-US477", @asset_element_rack, "477001");
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477003", "RACK2-US477", @asset_element_rack, "477001");
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477004", "RACK3-US477", @asset_element_rack, "477001");

/* UPSs */
insert into t_bios_asset_element (id_asset_element ,name , id_type, id_parent) values ("477005", "UPS1-US477", @asset_element_device, "477002");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477011", "477005", @asset_device_ups);
insert into t_bios_asset_element (id_asset_element ,name , id_type, id_parent) values ("477006", "UPS2-US477", @asset_element_device, "477003");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477012", "477006", @asset_device_ups);
insert into t_bios_asset_element (id_asset_element ,name , id_type, id_parent) values ("477007", "UPS3-US477", @asset_element_device, "477004");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477013", "477007", @asset_device_ups);

/* Servers */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477008", "SRV1-US477",  @asset_element_device, "477002");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477014", "477008", @asset_device_server);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477009", "SRV2-US477",  @asset_element_device, "477003");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477015", "477009", @asset_device_server);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values ("477010", "SRV3-US477",  @asset_element_device, "477004");
insert into t_bios_asset_device  (id_asset_device, id_asset_element, id_asset_device_type) values ("477016", "477010", @asset_device_server);

/* Powerchain */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    "477011",
    "477014",
    @asset_link_powerchain
);
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    "477012",
    "477015",
    @asset_link_powerchain
);
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    "477013",
    "477016",
    @asset_link_powerchain
);


insert into t_bios_discovered_device (name, id_device_type) values ("UPS1-US477", @device_ups);
insert into t_bios_discovered_device (name, id_device_type) values ("UPS2-US477", @device_ups);
insert into t_bios_discovered_device (name, id_device_type) values ("UPS3-US477", @device_ups);
insert into t_bios_discovered_device (name, id_device_type) values ("SRV1-US477", @device_server);
insert into t_bios_discovered_device (name, id_device_type) values ("SRV2-US477", @device_server);
insert into t_bios_discovered_device (name, id_device_type) values ("SRV3-US477", @device_server);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'UPS1-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS1-US477')
);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'UPS2-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS2-US477')
);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'UPS3-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS3-US477')
);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'SRV1-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV1-US477')
);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'SRV2-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV2-US477')
);
insert into t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES (
    (select id_discovered_device from t_bios_discovered_device where name = 'UPS1-US477'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV2-US477')
);
