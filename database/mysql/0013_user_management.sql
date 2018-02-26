/*
 *
 * Copyright (C) 2017-2018 Eaton
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

/*
*
* User management tables creations 
*
*
*/

/* User table */
CREATE TABLE IF NOT EXISTS t_user(
    uid             INTEGER UNSIGNED  NOT NULL,
    PRIMARY KEY (uid)
);

/* Vcard table */
CREATE TABLE IF NOT EXISTS t_vcard(
    id_vcard         INTEGER UNSIGNED  NOT NULL AUTO_INCREMENT,
    uid              INTEGER UNSIGNED  NOT NULL,
    keytag           VARCHAR(40)       NOT NULL, /* key  (i.e fullName, phone, email, etc.) */
    value            VARCHAR(255)      NOT NULL, /* value attached to the key */
    PRIMARY KEY (id_vcard),

    INDEX FK_USER_ELEMENT_idx (uid ASC),
    UNIQUE INDEX `UI_t_vcard` (`keytag`, `uid` ASC),

    CONSTRAINT FK_USER_ELEMENT
        FOREIGN KEY (uid)
        REFERENCES t_user (uid)
    ON DELETE RESTRICT
);

INSERT INTO t_user (uid) VALUES (1001);
INSERT INTO t_user (uid) VALUES (1002);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
