INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (1, "asset_crud_SUP_PARENT",2, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (2, "asset_crud_DEVICE",6, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (3, "asset_crud_GROUP",1, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (4, "asset_crud_DEVICE_src",6, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (5, "asset_crud_DEVICE_dest",6, NULL);

INSERT INTO t_bios_asset_device (id_asset_element, id_asset_device_type) VALUES (4, 2);
INSERT INTO t_bios_asset_device (id_asset_element, id_asset_device_type) VALUES (5, 3);
