/*
 *
 * Copyright (C) 2019 - 2020 Eaton
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
 * @file    02_02_00_00_fty-update.sql
 * @brief   Update of sql schema for 2.2.0 core.
 */


/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202011260001';
SET @bios_db_schema_filename = '02_02_00_00_fty-update.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

/* Update extended attribut key size from 40 char to 255 char */
ALTER TABLE t_bios_asset_ext_attributes MODIFY COLUMN keytag VARCHAR(255);


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
