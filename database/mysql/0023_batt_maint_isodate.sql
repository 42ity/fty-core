/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202108200001' ;
SET @bios_db_schema_filename = '0023_batt_maint_isodate.sql' ;

use box_utf8;


/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

/* Update existing battery.date records from "NUT date" (mm/dd/yyyy) to ISO Calendar date */
UPDATE t_bios_asset_ext_attributes
SET value = DATE_FORMAT(STR_TO_DATE(value, '%m/%d/%Y'), '%Y-%m-%d')
WHERE keytag = "battery.date" AND value LIKE '__/__/____';

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
