/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201709050001' ;
SET @bios_db_schema_filename = '0005_super_parent_names.sql' ;

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

DROP VIEW IF EXISTS v_bios_asset_element;
CREATE VIEW v_bios_asset_element AS
    SELECT  v1.id_asset_element AS id,
            v1.name,
            v1.id_type,
            v1.id_subtype,
            v1.id_parent,
            v2.id_type AS id_parent_type,
            v2.name AS parent_name,
            v1.status,
            v1.priority,
            v1.asset_tag
        FROM t_bios_asset_element v1
        LEFT JOIN  t_bios_asset_element v2
            ON (v1.id_parent = v2.id_asset_element) ;

DROP VIEW IF EXISTS v_bios_asset_element_super_parent;
CREATE VIEW v_bios_asset_element_super_parent AS
SELECT v1.id as id_asset_element,
       v1.id_parent AS id_parent1,
       v2.id_parent AS id_parent2,
       v3.id_parent AS id_parent3,
       v4.id_parent AS id_parent4,
       v5.id_parent AS id_parent5,
       v6.id_parent AS id_parent6,
       v1.parent_name AS name_parent1,
       v2.parent_name AS name_parent2,
       v3.parent_name AS name_parent3,
       v4.parent_name AS name_parent4,
       v5.parent_name AS name_parent5,
       v6.parent_name AS name_parent6,
       v1.name ,
       v7.name AS type_name,
       v7.id_asset_device_type,
       v1.status,
       v1.asset_tag,
       v1.priority,
       v1.id_type
FROM v_bios_asset_element v1
     LEFT JOIN v_bios_asset_element v2
        ON (v1.id_parent = v2.id)
     LEFT JOIN v_bios_asset_element v3
        ON (v2.id_parent = v3.id)
     LEFT JOIN v_bios_asset_element v4
        ON (v3.id_parent = v4.id)
     LEFT JOIN v_bios_asset_element v5
        ON (v4.id_parent = v5.id)
     LEFT JOIN v_bios_asset_element v6
        ON (v5.id_parent = v6.id)
     INNER JOIN t_bios_asset_device_type v7
        ON (v7.id_asset_device_type = v1.id_subtype);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
