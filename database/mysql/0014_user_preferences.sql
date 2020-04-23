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
 * @file    0014_user_preferences.sql
 * @brief   User preferences builtin data.
 */


/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201806140001';
SET @bios_db_schema_filename = '0014_user_preferences.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

\! /bin/rm -f /tmp/0013_um_envvars.sql
\! /usr/share/bios/scripts/generate_env_user4sql.sh -O /tmp/0013_um_envvars.sql
SOURCE /tmp/0013_um_envvars.sql;
\! /bin/rm -f /tmp/0013_um_envvars.sql

INSERT IGNORE INTO t_bios_agent_info(agent_name, info) VALUES
    (CAST(@ENV_ADMIN as CHAR(50)), '"preferences":{"email" : " ", "telephone" : " ", "organization" : " ", "date":"DDMMYYYY", "temperature":"C", "language":"en-us", "time":"24h"}'),
    (CAST(@ENV_MONITOR as CHAR(50)), '"preferences":{"email" : " ", "telephone" : " ", "organization" : " ", "date":"DDMMYYYY", "temperature":"C", "language":"en-us", "time":"24h"}');


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
