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
 * @file    0013_user_management.sql
 * @author: <a href="mailto:XavierMillieret@Eaton.com">Xavier, Millieret (E9902068)</a>
 * @brief   User management data base definition. 
 */


/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201802140001';
SET @bios_db_schema_filename = '0013_user_management.sql';

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

/*
*
* User management tables creations 
*
*
*/

/* User table */
CREATE TABLE IF NOT EXISTS t_um_user(
    id_user             INTEGER UNSIGNED  NOT NULL,
    PRIMARY KEY (id_user)
);

/* Entity attibute value table */

CREATE TABLE IF NOT EXISTS t_um_entity_attibute_value(
    id_entity        INTEGER UNSIGNED  NOT NULL AUTO_INCREMENT,
    id_user          INTEGER UNSIGNED  NOT NULL,
    keytag           VARCHAR(255)      NOT NULL, /* key tag  */
    value            VARCHAR(255)      NOT NULL, /* value attached to the key */
    PRIMARY KEY (id_entity),

    INDEX FK_UM_USER_ELEMENT_idx (id_user ASC),
    UNIQUE INDEX `t_um_entity_attibute_value` (`keytag`, `id_user` ASC),

    CONSTRAINT FK_UM_USER_ELEMENT
        FOREIGN KEY (id_user)
        REFERENCES t_um_user (id_user)
    ON DELETE RESTRICT
);

INSERT INTO t_um_user (id_user) VALUES (@ENV_ADMIN);
INSERT INTO t_um_user (id_user) VALUES (@ENV_MONITOR);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
