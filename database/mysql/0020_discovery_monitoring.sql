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
 * @file    0020_discovery_monitoring.sql
 * @brief   User preferences builtin data.
 */


/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '202003030002';
SET @bios_db_schema_filename = '0020_discovery_monitoring.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

/* NUT configuration type */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration_type(
    id_nut_configuration_type           INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
    configuration_name                  VARCHAR(255) NOT NULL,
    driver                              VARCHAR(255) NOT NULL,
    port                                VARCHAR(255) NOT NULL,
    PRIMARY KEY (id_nut_configuration_type)
);

INSERT IGNORE INTO t_bios_nut_configuration_type
(id_nut_configuration_type, configuration_name, driver, port)
VALUES
(1, 'NUT driver snmp-ups (SNMPv1)',                         'snmp-ups',     '${asset.ext.ip.1}'),
(2, 'NUT driver snmp-ups (SNMPv3)',                         'snmp-ups',     '${asset.ext.ip.1}'),
(3, 'NUT driver snmp-ups-dmf (SNMPv1)',                     'snmp-ups-dmf', '${asset.ext.ip.1}'),
(4, 'NUT driver snmp-ups-dmf (SNMPv3)',                     'snmp-ups-dmf', '${asset.ext.ip.1}'),
(5, 'NUT driver netxml-ups (XML-PDCv3)',                    'netxml-ups',   'http://${asset.ext.ip.1}'),
(6, 'NUT driver dummy-ups (repeater mode) with asset name', 'dummy-ups',    '${asset.ext.name}@${asset.ext.ip.1}');

/* NUT configuration */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration(
    id_nut_configuration                INTEGER UNSIGNED NOT NULL AUTO_INCREMENT,
    id_nut_configuration_type           INTEGER UNSIGNED NOT NULL,
    id_asset_element                    INTEGER UNSIGNED NOT NULL,
    priority                            INTEGER UNSIGNED NOT NULL,
    is_enabled                          BOOLEAN NOT NULL,
    is_working                          BOOLEAN NOT NULL,
    PRIMARY KEY (id_nut_configuration),

    CONSTRAINT UNIQUE (id_asset_element, priority),

    CONSTRAINT FK_NUT_CONFIGURATION_NUT_CONFIGURATION_TYPE
        FOREIGN KEY (id_nut_configuration_type)
        REFERENCES t_bios_nut_configuration_type (id_nut_configuration_type)
    ON DELETE RESTRICT,

    CONSTRAINT FK_NUT_CONFIGURATION_BIOS_ASSET_ELEMENT
        FOREIGN KEY (id_asset_element)
        REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE CASCADE
);

/* Tuples of (NUT configuration; secw document) */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration_secw_document(
    id_nut_configuration                INTEGER UNSIGNED NOT NULL,
    id_secw_document                    BINARY(16) NOT NULL,
    PRIMARY KEY (id_nut_configuration, id_secw_document),

    CONSTRAINT FK_NUT_CONFIGURATION_SECW_DOCUMENT_SECW_DOCUMENT
        FOREIGN KEY (id_secw_document)
        REFERENCES t_bios_secw_document (id_secw_document)
    ON DELETE RESTRICT,

    CONSTRAINT FK_NUT_CONFIGURATION_SECW_DOCUMENT_NUT_CONFIGURATION
        FOREIGN KEY (id_nut_configuration)
        REFERENCES t_bios_nut_configuration (id_nut_configuration)
    ON DELETE CASCADE
);

/* List of secw documents requirements for a NUT configuration type */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration_type_secw_document_type_requirements(
    id_nut_configuration_type           INTEGER UNSIGNED NOT NULL,
    id_secw_document_type               VARCHAR(32) NOT NULL,
    PRIMARY KEY (id_nut_configuration_type, id_secw_document_type),

    CONSTRAINT FK_NUTCONFTYPE_SECWDOCTYPE_REQUIREMENTS_SECWDOCTYPE
        FOREIGN KEY (id_secw_document_type)
        REFERENCES t_bios_secw_document_type (id_secw_document_type)
    ON DELETE RESTRICT,

    CONSTRAINT FK_NUTCONFTYPE_SECWDOCTYPE_REQUIREMENTS_NUTCONFTYPE
        FOREIGN KEY (id_nut_configuration_type)
        REFERENCES t_bios_nut_configuration_type (id_nut_configuration_type)
    ON DELETE CASCADE
);

INSERT IGNORE INTO t_bios_nut_configuration_type_secw_document_type_requirements
(id_nut_configuration_type, id_secw_document_type)
VALUES
(1, 'Snmpv1'),
(2, 'Snmpv3'),
(3, 'Snmpv1'),
(4, 'Snmpv3');

/* NUT configuration attribute */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration_attribute(
    id_nut_configuration                INTEGER UNSIGNED NOT NULL,
    keytag                              VARCHAR(255) NOT NULL,
    value                               VARCHAR(255) NOT NULL,
    PRIMARY KEY (id_nut_configuration, keytag),

    CONSTRAINT FK_NUT_CONFIGURATION_ATTRIBUTE_NUT_CONFIGURATION
        FOREIGN KEY (id_nut_configuration)
        REFERENCES t_bios_nut_configuration (id_nut_configuration)
    ON DELETE CASCADE
);

/* NUT configuration default attribute */
CREATE TABLE IF NOT EXISTS t_bios_nut_configuration_default_attribute(
    id_nut_configuration_type               INTEGER UNSIGNED NOT NULL,
    keytag                                  VARCHAR(255) NOT NULL,
    value                                   VARCHAR(255) NOT NULL,
    PRIMARY KEY (id_nut_configuration_type, keytag),

    CONSTRAINT FK_NUTCONF_DEFAULT_ATTRIBUTE_NUT_CONFTYPE
        FOREIGN KEY (id_nut_configuration_type)
        REFERENCES t_bios_nut_configuration_type (id_nut_configuration_type)
    ON DELETE CASCADE
);

INSERT IGNORE INTO t_bios_nut_configuration_default_attribute
(id_nut_configuration_type, keytag, value)
VALUES
(2, 'snmp_version', 'v3'),
(4, 'snmp_version', 'v3'),
(6, 'synchronous', 'yes');

/* NUT configuration default attribute view */
CREATE OR REPLACE VIEW v_conf_default_attribute AS
SELECT config.id_asset_element as id_asset_element, config.id_nut_configuration as id_nut_configuration, conf_def_attr.keytag as keytag, conf_def_attr.value as value, config.priority as priority, config.is_enabled as is_enabled, config.is_working as is_working
FROM t_bios_nut_configuration config
INNER JOIN t_bios_nut_configuration_default_attribute conf_def_attr
ON conf_def_attr.id_nut_configuration_type = config.id_nut_configuration_type
UNION SELECT config.id_asset_element as id_asset_element, config.id_nut_configuration as id_nut_configuration, 'driver' as keytag, confType.driver as value, config.priority as priority, config.is_enabled as is_enabled, config.is_working as is_working
FROM t_bios_nut_configuration_type confType JOIN t_bios_nut_configuration config
ON confType.id_nut_configuration_type = config.id_nut_configuration_type
UNION SELECT config.id_asset_element as id_asset_element, config.id_nut_configuration as id_nut_configuration, 'port' as keytag, confType.port as value, config.priority as priority, config.is_enabled as is_enabled, config.is_working as is_working
FROM t_bios_nut_configuration_type confType JOIN t_bios_nut_configuration config
ON confType.id_nut_configuration_type = config.id_nut_configuration_type
ORDER BY id_asset_element, priority, id_nut_configuration, keytag;

/* NUT configuration device attribute view */
CREATE OR REPLACE VIEW v_conf_device_attribute AS
SELECT config.id_asset_element as id_asset_element, config.id_nut_configuration as id_nut_configuration, conf_attr.keytag as keytag, conf_attr.value as value, config.priority as priority, config.is_enabled as is_enabled, config.is_working as is_working
FROM t_bios_nut_configuration config
INNER JOIN t_bios_nut_configuration_attribute conf_attr
ON conf_attr.id_nut_configuration = config.id_nut_configuration
ORDER BY id_asset_element, priority, id_nut_configuration, keytag;

/* NUT configuration attribute view */
CREATE OR REPLACE VIEW v_conf_attribute AS
SELECT * FROM v_conf_default_attribute
UNION
SELECT * FROM v_conf_device_attribute
ORDER BY id_asset_element, priority, id_nut_configuration, keytag;

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
