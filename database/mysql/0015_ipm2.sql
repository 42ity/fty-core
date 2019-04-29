/*--------------------- 15 IPM2  ------------------ */

SET @bios_db_schema_version = '201904250001';
SET @bios_db_schema_filename = '0015_ipm2.sql';

use box_utf8;

/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);

ALTER TABLE t_bios_asset_element ADD id_secondary varchar(255) NULL;
ALTER TABLE t_bios_asset_link    ADD id_secondary varchar(255) NULL;

CREATE VIEW v_bios_asset_subelement_type AS SELECT id_asset_device_type AS id_asset_element_subtype, name FROM t_bios_asset_device_type;

/* Create 't_bios_asset_link_attributes' table */
DROP TABLE IF EXISTS t_bios_asset_link_attributes;
CREATE TABLE t_bios_asset_link_attributes (
  id_asset_link_attribute INT UNSIGNED NOT NULL AUTO_INCREMENT,
  keytag                  varchar(40)  NOT NULL,
  value                   varchar(255) NOT NULL,
  id_link                 INT UNSIGNED NOT NULL,
  read_only               TINYINT      NOT NULL DEFAULT 0,

  PRIMARY KEY (id_asset_link_attribute),

  INDEX  FK_ASSET_LINK_ATTR_idx (id_link ASC),
  UNIQUE KEY UI_t_bios_asset_link_attributes (keytag, id_link ASC),

  CONSTRAINT FK_ASSET_LINK_ATTR
    FOREIGN KEY (id_link) 
    REFERENCES t_bios_asset_link (id_link)
    ON DELETE RESTRICT
);

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag, timestamp, filename, version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;

