/*
 *
 * Copyright (C) 2020 Eaton
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
 * @file    0019_security_wallet.sql
 * @brief   User preferences builtin data.
 */


/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202003030001';
SET @bios_db_schema_filename = '0019_security_wallet.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

DELIMITER //

DROP FUNCTION IF EXISTS BIN_TO_UUID;
CREATE FUNCTION BIN_TO_UUID(b BINARY(16))
RETURNS CHAR(36)
BEGIN
   DECLARE hexStr CHAR(32);
   SET hexStr = HEX(b);
   RETURN LOWER(CONCAT(
        SUBSTR(hexStr, 1, 8), '-',
        SUBSTR(hexStr, 9, 4), '-',
        SUBSTR(hexStr, 13, 4), '-',
        SUBSTR(hexStr, 17, 4), '-',
        SUBSTR(hexStr, 21)));
END //

DROP FUNCTION IF EXISTS UUID_TO_BIN;
CREATE FUNCTION UUID_TO_BIN(uuid CHAR(36))
RETURNS BINARY(16)
BEGIN
    RETURN UNHEX(REPLACE(uuid, '-', ''));
END //

DELIMITER ;

GRANT EXECUTE ON FUNCTION box_utf8.BIN_TO_UUID TO `bios-rw`@`localhost`;
GRANT EXECUTE ON FUNCTION box_utf8.UUID_TO_BIN TO `bios-rw`@`localhost`;

/* Security document type */
CREATE TABLE IF NOT EXISTS t_bios_secw_document_type(
    id_secw_document_type               VARCHAR(32) NOT NULL,
    PRIMARY KEY (id_secw_document_type)
);

INSERT IGNORE INTO t_bios_secw_document_type
(id_secw_document_type)
VALUES
('Snmpv1'),
('Snmpv3'),
('UserAndPassword'),
('InternalCertificate'),
('ExternalCertificate');

/* Security document proxy object */
CREATE TABLE IF NOT EXISTS t_bios_secw_document(
    id_secw_document                    BINARY(16) NOT NULL,
    id_secw_document_type               VARCHAR(32) NOT NULL,
    PRIMARY KEY (id_secw_document),

    CONSTRAINT FK_SECW_DOCUMENT_SECW_DOCUMENT_TYPE
        FOREIGN KEY (id_secw_document_type)
        REFERENCES t_bios_secw_document_type (id_secw_document_type)
    ON DELETE RESTRICT
);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
