/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201709200001' ;
SET @bios_db_schema_filename = '0011_default_rc.sql' ;

use box_utf8;


/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

SET @sql_mode_backup = (SELECT @@sql_mode);
SET sql_mode='NO_AUTO_VALUE_ON_ZERO';
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_subtype, id_parent, status, priority, asset_tag) VALUES (0, 'rackcontroller-0', (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "device" LIMIT 1), (SELECT id_asset_device_type FROM t_bios_asset_device_type WHERE name = "rack controller" LIMIT 1), NULL, 'active', 1, NULL);
INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('name', 'IPC 3000', 0, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
/* not required INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('location_u_pos', '1', 0, 0), ('u_size', '1', 0, 0), ('ip.1', '0.0.0.0', 0, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute); */
/* not required INSERT INTO t_bios_asset_link (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) VALUES (0, 0, (SELECT id_asset_link_type FROM t_bios_asset_link_type WHERE name = "power chain" LIMIT 1),   NULL, NULL); */
INSERT INTO t_bios_discovered_device (name, id_device_type) VALUES ('IPC 3000', (SELECT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1)) ON DUPLICATE KEY UPDATE id_discovered_device = LAST_INSERT_ID(id_discovered_device);
INSERT INTO t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES ((SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'IPC 3000' AND id_device_type = (SELECT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1) LIMIT 1), 0);
SET sql_mode = (SELECT @sql_mode_backup);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
