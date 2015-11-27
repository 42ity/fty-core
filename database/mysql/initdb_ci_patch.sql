/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201510150001' ;
SET @bios_db_schema_filename = 'initdb_ci_patch.sql' ;

use box_utf8

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;


alter table t_bios_asset_element modify column asset_tag VARCHAR(50) DEFAULT "1234567890";
#drop index if exists UI_t_bios_asset_element_ASSET_TAG on t_bios_asset_element;


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
