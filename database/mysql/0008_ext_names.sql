/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201703090008' ;
SET @bios_db_schema_filename = '0008_ext_names.sql' ;

use box_utf8;


/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

/* copy asset.name to ext["name"] if does not already exists*/
INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element) SELECT "name", asset.name, asset.id_asset_element FROM t_bios_asset_element AS asset WHERE asset.id_asset_element NOT in (SELECT id_asset_element FROM t_bios_asset_ext_attributes AS ext WHERE ext.keytag="name");

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
