/*
 *
 * Copyright (C) 2017 - 2020 Eaton
 *
 * This software is confidential and licensed under Eaton Proprietary License
 * (EPL or EULA).
 *
 * This software is not authorized to be used, duplicated or disclosed to
 * anyone without the prior written permission of Eaton.
 * Limitations, restrictions and exclusions of the Eaton applicable standard
 * terms and conditions, such as its EPL and EULA, apply.
 *
 */

/*
 * @file    0019_device_type_extension.sql
 * @brief   Composite Power System related assets and devices
 * @note    Pass just before IT fall scripts (ipm_it_fall_virtu.sql)
 */

/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202009240001';
SET @bios_db_schema_filename = '0019_device_type_extension.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;


/* cops (composite power system) asset */
INSERT INTO t_bios_asset_element_type (name) VALUES ("cops");
/* pcu device (parallel control unit, as a marker of ups parallel system) */
INSERT INTO t_bios_asset_device_type (name) VALUES ("pcu");

/* Update extended attribut key size from 40 char to 255 char */
ALTER TABLE t_bios_asset_ext_attributes MODIFY COLUMN keytag VARCHAR(255);


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
