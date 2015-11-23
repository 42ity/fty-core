/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201510220001' ;
SET @bios_db_schema_filename = 'initdb_ci_patch_2.sql' ;

use box_utf8

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;


alter table t_bios_asset_element add UNIQUE INDEX `UI_t_bios_asset_element_ASSET_TAG` (`asset_tag`  ASC);


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
