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
 * @file    0026_measurement_value_i64.sql
 * @brief   Increase t_bios_measurement value (aggregated metrics) from int32 (INT) to int64 (BIGINT)
 * @note    Pass just before IT fall scripts (ipm_it_fall_virtu.sql)
 */

/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202202170001';
SET @bios_db_schema_filename = '0026_measurement_value_i64.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;


/* t_bios_measurement.value: int32 (INT) -> int64 (BIGINT) */
ALTER TABLE t_bios_measurement MODIFY COLUMN t_bios_measurement.value BIGINT;


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
