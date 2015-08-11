use box_utf8;

INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (1, "asset_crud_SUP_PARENT",2, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (2, "asset_crud_DEVICE",6, NULL);
INSERT INTO t_bios_asset_element(id_asset_element, name, id_type, id_parent) VALUES (3, "asset_crud_GROUP",1, NULL);

INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_subtype, id_parent) VALUES (4, "asset_crud_DEVICE_src",6, 2, NULL);
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_subtype, id_parent) VALUES (5, "asset_crud_DEVICE_dest",6, 3, NULL);

/* for aupdate element*/
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_subtype, id_parent, business_crit, status, priority) VALUES (6, "asset_crud_DEVICE_update",6, 3, 1, 0, "nonactive", 3);
