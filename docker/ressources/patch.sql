use box_utf8
insert into t_bios_discovered_device (name, id_device_type) values ("UPS1-LAB", 2);
insert into t_bios_discovered_device (name, id_device_type) values ("ePDU-LAB", 3);
insert into t_bios_discovered_device (name, id_device_type) values ("ups", 2);
insert into t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) values (1,7);
insert into t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) values (5,6);
insert into t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) values (4,22);
insert into t_bios_monitor_asset_relation (id_discovered_device,id_asset_element) values (5,24);

