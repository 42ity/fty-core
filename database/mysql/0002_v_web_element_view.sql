/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201603180001' ;
SET @bios_db_schema_filename = '0002_v_web_element_view.sql' ;

use box_utf8;


/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;


DROP VIEW IF EXISTS v_web_element;
CREATE VIEW v_web_element AS
    SELECT
        t1.id_asset_element AS id,
        t1.name,
        t1.id_type,
        v3.name AS type_name,
        t1.id_subtype AS subtype_id,
        v4.name AS subtype_name,
        t1.id_parent,
        t2.id_type AS id_parent_type,
        t2.name AS parent_name,
        t1.status,
        t1.priority,
        t1.asset_tag
    FROM
        t_bios_asset_element t1
        LEFT JOIN t_bios_asset_element t2
            ON (t1.id_parent = t2.id_asset_element)
        LEFT JOIN v_bios_asset_element_type v3
            ON (t1.id_type = v3.id)
        LEFT JOIN t_bios_asset_device_type v4
            ON (v4.id_asset_device_type = t1.id_subtype);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
