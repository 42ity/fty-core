/* NOTE FOR DEVELOPERS: The schema-version must be changed (e.g. increased)
 * whenever you change table/view structures, so upgrade/recreation can be
 * triggered by bios-db-init service (whenever we figure out how to really
 * upgrade, see BIOS-1332). CURRENT implementation of "db-init" just refuses
 * to start the service and its dependents if the strings in database and
 * supplied copy of this SQL file are different in any manner. */
/* Gentleman's agreement on thios arbitrary string is that it is YYYYMMDDNNNN
 * sort of timestamp + number of change within a day, so it always increases
 * and we can discern upgrades vs. downgrades at later stage in development */
/* Other schema files, e.g. for additional modules, are encouraged to copy
 * this pattern and also add their versions and appropriate filenames when
 * they begin and finish to initialize their schems bits. The "db-init" script
 * will import and/or validate any *.sql file in its resource directory. */
SET @bios_db_schema_version = '201510150002' ;
SET @bios_db_schema_filename = 'initdb.sql' ;

DROP DATABASE IF EXISTS box_utf8;
CREATE DATABASE IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

USE box_utf8;

SET GLOBAL time_zone='+00:00';

/* work around smart insert without duplicates*/
CREATE TABLE IF NOT EXISTS t_empty (id TINYINT);
INSERT INTO t_empty values (1);

/* Values added in the beginning and end of SQL import to validate it succeeded */
/* Note: theoretically we should support upgrades, so this table would have
 * several begin-finish entries with different versions and timestamps. So both
 * in the informative SELECT below and in the "db-init" script we take care to
 * pick only the latest entry with each tag for report or comparison, and do not
 * require to destroy the table. */
CREATE TABLE IF NOT EXISTS t_bios_schema_version(
    id               INTEGER UNSIGNED  NOT NULL AUTO_INCREMENT,
    tag              VARCHAR(16)       NOT NULL, /* 'begin-import' or 'finish-import' */
    filename         VARCHAR(32)       NOT NULL, /* base filename.sql to support multiple SQLs with their versions */
    timestamp        BIGINT            NOT NULL, /* timestamp of the entry, just in case */
    version          VARCHAR(16)       NOT NULL, /* arbitrary string, e.g. YYYYMMDDNNNN */
    PRIMARY KEY(id)
);
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

DROP TABLE if exists t_bios_monitor_asset_relation;
drop table if exists t_bios_measurement;
drop table if exists t_bios_measurement_topic;
drop table if exists t_bios_discovered_device;
drop table if exists t_bios_device_type;
drop table if exists t_bios_alert_device;
drop table if exists t_bios_alert;

CREATE TABLE t_bios_measurement_topic(
    id               INTEGER UNSIGNED  NOT NULL AUTO_INCREMENT,
    device_id        INTEGER UNSIGNED  NOT NULL DEFAULT '0',
    units            VARCHAR(10)       NOT NULL,
    topic            VARCHAR(255)      NOT NULL,
    PRIMARY KEY(id),

    INDEX(device_id,topic,units),
    UNIQUE INDEX `UI_t_bios_measurement_topic` (`device_id`, `units`, `topic`  ASC)

);

CREATE TABLE t_bios_measurement (
    id            BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,
    timestamp     BIGINT              NOT NULL,
    value         INTEGER             NOT NULL,
    scale         SMALLINT            NOT NULL,
    topic_id      INTEGER UNSIGNED    NOT NULL,

    PRIMARY KEY(id),

    INDEX(topic_id),
    INDEX(timestamp),
    UNIQUE INDEX `UI_t_bios_measurement` (`timestamp`, `topic_id`  ASC),

    FOREIGN KEY(topic_id)
        REFERENCES t_bios_measurement_topic(id)
        ON DELETE CASCADE
) ENGINE=Aria;

CREATE TABLE t_bios_device_type(
    id_device_type      TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name                VARCHAR(50)      NOT NULL,

    PRIMARY KEY(id_device_type),

    UNIQUE INDEX `UI_t_bios_device_type_name` (`name` ASC)

) AUTO_INCREMENT = 1;

CREATE TABLE t_bios_discovered_device(
    id_discovered_device    SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name                    VARCHAR(50)       NOT NULL,
    id_device_type          TINYINT UNSIGNED  NOT NULL,

    PRIMARY KEY(id_discovered_device),

    INDEX(id_device_type),

    UNIQUE INDEX `UI_t_bios_discovered_device_name` (`name` ASC),

    FOREIGN KEY(id_device_type)
	REFERENCES t_bios_device_type(id_device_type)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_alert(
    id           INT UNSIGNED     NOT NULL AUTO_INCREMENT,
    rule_name    VARCHAR(50)      NOT NULL,
    date_from    BIGINT         NOT NULL,
    priority     TINYINT UNSIGNED NOT NULL,
    state        TINYINT UNSIGNED NOT NULL,
    description  VARCHAR(255),
    date_till    BIGINT,
    notification TINYINT         NOT NULL DEFAULT 0,
    dc_id        INT UNSIGNED     , /*NOT NULL,*/

    PRIMARY KEY(id),

    INDEX(rule_name)
);

CREATE TABLE t_bios_alert_device(
    id          INT UNSIGNED        NOT NULL AUTO_INCREMENT,
    alert_id    INT UNSIGNED        NOT NULL,
    device_id   SMALLINT UNSIGNED   NOT NULL,

    PRIMARY KEY(id),
    UNIQUE INDEX `UI_t_bios_alert_device` (`alert_id`, device_id ASC),

    INDEX(alert_id),
    INDEX(device_id),

    FOREIGN KEY(alert_id)
        REFERENCES t_bios_alert(id)
        ON DELETE RESTRICT,

    FOREIGN KEY(device_id)
        REFERENCES t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_agent_info(
    id          SMALLINT UNSIGNED   NOT NULL AUTO_INCREMENT,
    agent_name  VARCHAR(50)         NOT NULL,
    info        BLOB                NOT NULL,

    PRIMARY KEY(id),
    UNIQUE INDEX `UI_t_bios_agent_info` (`agent_name` ASC)

);

DROP TABLE if exists t_bios_asset_link;
DROP TABLE if exists t_bios_asset_link_type;
DROP TABLE if exists t_bios_asset_device_type;
DROP TABLE if exists t_bios_asset_ext_attributes;
DROP TABLE if exists t_bios_asset_group_relation;
DROP TABLE if exists t_bios_asset_group;
DROP TABLE if exists t_bios_asset_group_type;
DROP TABLE if exists t_bios_asset_element;
DROP TABLE if exists t_bios_asset_element_type;

CREATE TABLE t_bios_asset_element_type (
  id_asset_element_type TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
  name                  VARCHAR(50)      NOT NULL,

  PRIMARY KEY (id_asset_element_type),

  UNIQUE INDEX `UI_t_bios_asset_element_type` (`name` ASC)

) AUTO_INCREMENT = 1;

CREATE TABLE t_bios_asset_device_type(
  id_asset_device_type TINYINT UNSIGNED   NOT NULL AUTO_INCREMENT,
  name                 VARCHAR(50)        NOT NULL,

  PRIMARY KEY (id_asset_device_type),
  UNIQUE INDEX `UI_t_bios_asset_device_type` (`name` ASC)

);

/* ATTENTION: don't change N_A id. Here it is used*/
CREATE TABLE t_bios_asset_element (
  id_asset_element  INT UNSIGNED        NOT NULL AUTO_INCREMENT,
  name              VARCHAR(50)         NOT NULL,
  id_type           TINYINT UNSIGNED    NOT NULL,
  id_subtype        TINYINT UNSIGNED    NOT NULL DEFAULT 11,
  id_parent         INT UNSIGNED,
  status            VARCHAR(9)          NOT NULL DEFAULT "nonactive",
  priority          TINYINT             NOT NULL DEFAULT 5,
  business_crit     TINYINT             NOT NULL DEFAULT 0,
  asset_tag         VARCHAR(50),

  PRIMARY KEY (id_asset_element),

  INDEX FK_ASSETELEMENT_ELEMENTTYPE_idx (id_type   ASC),
  INDEX FK_ASSETELEMENT_ELEMENTSUBTYPE_idx (id_subtype   ASC),
  INDEX FK_ASSETELEMENT_PARENTID_idx    (id_parent ASC),
  UNIQUE INDEX `UI_t_bios_asset_element_NAME` (`name` ASC),
  INDEX `UI_t_bios_asset_element_ASSET_TAG` (`asset_tag`  ASC),

  CONSTRAINT FK_ASSETELEMENT_ELEMENTTYPE
    FOREIGN KEY (id_type)
    REFERENCES t_bios_asset_element_type (id_asset_element_type)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETELEMENT_ELEMENTSUBTYPE
    FOREIGN KEY (id_subtype)
    REFERENCES t_bios_asset_device_type (id_asset_device_type)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETELEMENT_PARENTID
    FOREIGN KEY (id_parent)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);

CREATE TABLE t_bios_asset_group_relation (
  id_asset_group_relation INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_asset_group          INT UNSIGNED NOT NULL,
  id_asset_element        INT UNSIGNED NOT NULL,

  PRIMARY KEY (id_asset_group_relation),

  INDEX FK_ASSETGROUPRELATION_ELEMENT_idx (id_asset_element ASC),
  INDEX FK_ASSETGROUPRELATION_GROUP_idx   (id_asset_group   ASC),

  UNIQUE INDEX `UI_t_bios_asset_group_relation` (`id_asset_group`, `id_asset_element` ASC),

  CONSTRAINT FK_ASSETGROUPRELATION_ELEMENT
    FOREIGN KEY (id_asset_element)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETGROUPRELATION_GROUP
    FOREIGN KEY (id_asset_group)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);


CREATE TABLE t_bios_asset_link_type(
  id_asset_link_type   TINYINT UNSIGNED   NOT NULL AUTO_INCREMENT,
  name                 VARCHAR(50)        NOT NULL,

  PRIMARY KEY (id_asset_link_type),
  UNIQUE INDEX `UI_t_bios_asset_link_type_name` (`name` ASC)

);

CREATE TABLE t_bios_asset_link (
  id_link               INT UNSIGNED        NOT NULL AUTO_INCREMENT,
  id_asset_device_src   INT UNSIGNED        NOT NULL,
  src_out               CHAR(4),
  id_asset_device_dest  INT UNSIGNED        NOT NULL,
  dest_in               CHAR(4),
  id_asset_link_type    TINYINT UNSIGNED    NOT NULL,

  PRIMARY KEY (id_link),

  INDEX FK_ASSETLINK_SRC_idx  (id_asset_device_src    ASC),
  INDEX FK_ASSETLINK_DEST_idx (id_asset_device_dest   ASC),
  INDEX FK_ASSETLINK_TYPE_idx (id_asset_link_type     ASC),

  CONSTRAINT FK_ASSETLINK_SRC
    FOREIGN KEY (id_asset_device_src)
    REFERENCES t_bios_asset_element(id_asset_element)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETLINK_DEST
    FOREIGN KEY (id_asset_device_dest)
    REFERENCES t_bios_asset_element(id_asset_element)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETLINK_TYPE
    FOREIGN KEY (id_asset_link_type)
    REFERENCES t_bios_asset_link_type(id_asset_link_type)
    ON DELETE RESTRICT

);

CREATE TABLE t_bios_asset_ext_attributes(
  id_asset_ext_attribute    INT UNSIGNED NOT NULL AUTO_INCREMENT,
  keytag                    VARCHAR(40)  NOT NULL,
  value                     VARCHAR(255) NOT NULL,
  id_asset_element          INT UNSIGNED NOT NULL,
  read_only                 TINYINT      NOT NULL DEFAULT 0,

  PRIMARY KEY (id_asset_ext_attribute),

  INDEX FK_ASSETEXTATTR_ELEMENT_idx (id_asset_element ASC),
  UNIQUE INDEX `UI_t_bios_asset_ext_attributes` (`keytag`, `id_asset_element` ASC),

  CONSTRAINT FK_ASSETEXTATTR_ELEMENT
    FOREIGN KEY (id_asset_element)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);

CREATE TABLE t_bios_monitor_asset_relation(
    id_ma_relation        INT UNSIGNED      NOT NULL AUTO_INCREMENT,
    id_discovered_device  SMALLINT UNSIGNED NOT NULL,
    id_asset_element      INT UNSIGNED      NOT NULL,

    PRIMARY KEY(id_ma_relation),

    INDEX(id_discovered_device),
    INDEX(id_asset_element),

    FOREIGN KEY (id_discovered_device)
        REFERENCEs t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT,

    FOREIGN KEY (id_asset_element)
        REFERENCEs t_bios_asset_element(id_asset_element)
        ON DELETE RESTRICT

);

drop view if exists v_bios_device_type;
drop view if exists v_bios_discovered_device;

create view v_bios_device_type as select id_device_type id, name from t_bios_device_type;

create view v_bios_discovered_device as select id_discovered_device id, name , id_device_type from t_bios_discovered_device;

DROP VIEW IF EXISTS v_bios_agent_info;
CREATE VIEW v_bios_agent_info AS
    SELECT id, info, agent_name
    FROM t_bios_agent_info;


DROP view if exists v_bios_asset_device;
DROP view if exists v_bios_asset_device_type;
DROP view if exists v_bios_asset_ext_attributes;
DROP view if exists v_bios_asset_group_relation;
DROP view if exists v_bios_asset_link_type;
DROP view if exists v_bios_asset_link;
DROP VIEW IF EXISTS v_bios_monitor_asset_relation;

DROP VIEW IF EXISTS v_bios_alert_device;
CREATE VIEW v_bios_alert_device AS
    SELECT
        id,
        alert_id,
        device_id
    FROM
        t_bios_alert_device;


DROP VIEW IF EXISTS v_bios_alert;
CREATE VIEW v_bios_alert AS
    SELECT
        id,
        rule_name,
        date_from,
        priority,
        state,
        description,
        date_till,
        notification
    FROM
        t_bios_alert;

DROP VIEW IF EXISTS v_bios_asset_element_type;
CREATE VIEW v_bios_asset_element_type AS
    SELECT
        t1.id_asset_element_type AS id,
        t1.name
    FROM
        t_bios_asset_element_type t1;

DROP VIEW IF EXISTS v_bios_asset_device;
CREATE VIEW v_bios_asset_device AS
    SELECT  t1.id_asset_element,
            t2.id_asset_device_type,
            t2.name
    FROM t_bios_asset_element t1
        LEFT JOIN t_bios_asset_device_type t2
        ON (t1.id_subtype = t2.id_asset_device_type);

DROP VIEW IF EXISTS v_web_element;
CREATE VIEW v_web_element AS
    SELECT
        t1.id_asset_element AS id,
        t1.name,
        t1.id_type,
        v3.name AS type_name,
        t1.id_subtype AS subtype_id,
        v4.name AS subtype_name,
        t1.id_parent,
        t2.id_type AS id_parent_type,
        t1.business_crit,
        t1.status,
        t1.priority,
        t1.asset_tag
    FROM
        t_bios_asset_element t1
        LEFT JOIN t_bios_asset_element t2
            ON (t1.id_parent = t2.id_asset_element)
        LEFT JOIN v_bios_asset_element_type v3
            ON (t1.id_type = v3.id)
        LEFT JOIN t_bios_asset_device_type v4
            ON (v4.id_asset_device_type = t1.id_subtype);


/* for REST API: /asset/all */
DROP VIEW IF EXISTS v_web_alert_all;
CREATE VIEW v_web_alert_all AS
    SELECT
        v1.id,
        v1.rule_name,
        v1.date_from,
        v1.priority,
        v1.state,
        v1.description,
        v1.date_till,
        v1.notification,
        t3.id_asset_element,
        v4.type_name,
        v4.subtype_name
    FROM
        v_bios_alert v1
        LEFT JOIN v_bios_alert_device v2
            ON v1.id = v2.alert_id
        LEFT JOIN t_bios_monitor_asset_relation t3
            ON v2.device_id = t3.id_discovered_device
        LEFT JOIN v_web_element v4
            ON t3.id_asset_element = v4.id
    ORDER BY v1.id;

DROP VIEW IF EXISTS v_bios_asset_link;
CREATE VIEW v_bios_asset_link AS
    SELECT  v1.id_link,
            v1.src_out,
            v1.dest_in,
            v1.id_asset_link_type,
            v1.id_asset_device_src AS id_asset_element_src,
            v1.id_asset_device_dest AS id_asset_element_dest
    FROM t_bios_asset_link v1;

DROP VIEW  if exists v_web_asset_link;
CREATE VIEW v_web_asset_link AS
    SELECT
        v1.id_link,
        v1.id_asset_link_type,
        t3.name AS link_name,
        v1.id_asset_element_src,
        t1.name AS src_name,
        v1.id_asset_element_dest,
        t2.name AS dest_name,
        v1.src_out,
        v1.dest_in
    FROM
         v_bios_asset_link v1
    JOIN t_bios_asset_element t1
        ON v1.id_asset_element_src=t1.id_asset_element
    JOIN t_bios_asset_element t2
        ON v1.id_asset_element_dest=t2.id_asset_element
    JOIN t_bios_asset_link_type t3
        ON v1.id_asset_link_type=t3.id_asset_link_type;


CREATE VIEW v_bios_asset_link_topology AS
    SELECT  v1.src_out,
            v1.dest_in,
            v1.id_asset_link_type,
            v1.id_asset_device_src AS id_asset_element_src,
            v4.name AS src_name,
            v1.id_asset_device_dest AS id_asset_element_dest,
            v5.name AS dest_name,
            v6.name AS src_type_name,
            v7.name AS dest_type_name,
            v6.id_asset_device_type AS src_type_id,
            v7.id_asset_device_type AS dest_type_id
    FROM t_bios_asset_link v1
        LEFT JOIN t_bios_asset_element v4
        ON (v4.id_asset_element = v1.id_asset_device_src)
        LEFT JOIN t_bios_asset_element v5
        ON (v5.id_asset_element = v1.id_asset_device_dest)
        LEFT JOIN t_bios_asset_device_type v6
        ON (v4.id_subtype = v6.id_asset_device_type)
        LEFT JOIN t_bios_asset_device_type v7
        ON (v5.id_subtype = v7.id_asset_device_type);

create view v_bios_asset_device_type as select id_asset_device_type as id, name from t_bios_asset_device_type ;
create view v_bios_asset_ext_attributes as select * from t_bios_asset_ext_attributes ;
create view v_bios_asset_group_relation as select * from t_bios_asset_group_relation ;
DROP VIEW IF EXISTS v_bios_asset_element;
CREATE VIEW v_bios_asset_element AS
    SELECT  v1.id_asset_element AS id,
            v1.name,
            v1.id_type,
            v1.id_subtype,
            v1.id_parent,
            v2.id_type AS id_parent_type,
            v1.business_crit,
            v1.status,
            v1.priority,
            v1.asset_tag
        FROM t_bios_asset_element v1
        LEFT JOIN  t_bios_asset_element v2
            ON (v1.id_parent = v2.id_asset_element) ;

create view v_bios_monitor_asset_relation as select * from t_bios_monitor_asset_relation;

CREATE VIEW v_bios_asset_element_super_parent AS
SELECT v1.id_asset_element,
       v1.id_parent AS id_parent1,
       v2.id_parent AS id_parent2,
       v3.id_parent AS id_parent3,
       v4.id_parent AS id_parent4,
       v5.id_parent AS id_parent5,
       v1.name ,
       v6.name AS type_name,
       v6.id_asset_device_type,
       v1.status,
       v1.asset_tag,
       v1.priority,
       v1.business_crit,
       v1.id_type
FROM t_bios_asset_element v1
     LEFT JOIN t_bios_asset_element v2
        ON (v1.id_parent = v2.id_asset_element)
     LEFT JOIN t_bios_asset_element v3
        ON (v2.id_parent = v3.id_asset_element)
     LEFT JOIN t_bios_asset_element v4
        ON (v3.id_parent=v4.id_asset_element)
     LEFT JOIN t_bios_asset_element v5
        ON (v4.id_parent=v5.id_asset_element)
     INNER JOIN t_bios_asset_device_type v6
        ON (v6.id_asset_device_type = v1.id_subtype);

CREATE VIEW v_bios_measurement AS
SELECT t1.id,
       t1.timestamp,
       t1.value,
       t1.scale,
       t2.device_id,
       t2.units,
       t2.topic,
       t2.id AS topic_id
FROM t_bios_measurement t1
    LEFT JOIN t_bios_measurement_topic t2 ON
        (t1.topic_id = t2.id);

/* Selects the last known value during last 10 = 5*2 minutes  */
/* 5 minuts is polling interval
/* TODO: need to be configurable*/
DROP VIEW IF EXISTS v_web_measurement_last_10m;
DROP VIEW IF EXISTS v_web_measurement_lastdate_10m;
DROP VIEW IF EXISTS v_web_measurement_10m;

DROP VIEW IF EXISTS v_web_measurement_last_24h;
DROP VIEW IF EXISTS v_web_measurement_lastdate_24h;
DROP VIEW IF EXISTS v_web_measurement_24h;

/* ========================================================= */

CREATE VIEW v_web_measurement_10m AS
SELECT v.id,
        v.device_id,
        v.timestamp,
        v.topic,
        v.topic_id,
        v.value,
        v.scale
FROM v_bios_measurement v
WHERE v.timestamp > UNIX_TIMESTAMP() - 10*60;

CREATE VIEW v_web_measurement_lastdate_10m AS
SELECT max(p.timestamp) maxdate,
       p.device_id,
       p.topic_id
FROM v_web_measurement_10m p
GROUP BY p.topic_id, p.device_id;

CREATE VIEW v_web_measurement_last_10m AS
SELECT  v.id,
        v.device_id,
        v.timestamp,
        v.topic,
        v.topic_id,
        v.value,
        v.scale
FROM       v_web_measurement_10m v
INNER JOIN v_web_measurement_lastdate_10m grp
     ON ( v.timestamp = grp.maxdate  AND
        v.topic_id = grp.topic_id );


CREATE VIEW v_web_measurement_24h AS
SELECT v.id,
        v.device_id,
        v.timestamp,
        v.topic,
        v.topic_id,
        v.value,
        v.scale
FROM v_bios_measurement v
WHERE v.timestamp > UNIX_TIMESTAMP() - 24*60*60 - 25*60;

CREATE VIEW v_web_measurement_lastdate_24h AS
SELECT max(p.timestamp) maxdate,
       p.device_id,
       p.topic_id
FROM v_web_measurement_24h p
GROUP BY p.topic_id, p.device_id;

CREATE VIEW v_web_measurement_last_24h AS
SELECT  v.id,
        v.device_id,
        v.timestamp,
        v.topic,
        v.topic_id,
        v.value,
        v.scale
FROM       v_web_measurement_24h v
INNER JOIN v_web_measurement_lastdate_24h grp
     ON ( v.timestamp = grp.maxdate  AND
        v.topic_id = grp.topic_id );

/* ========================================================= */

CREATE VIEW v_bios_measurement_topic AS
SELECT *
FROM   t_bios_measurement_topic;


/* *************************************************************************** */
/* **********************          INSERTIONS          *********************** */
/* *************************************************************************** */

/* t_bios_device_type */
INSERT INTO t_bios_device_type (name) VALUES ("not_classified");
INSERT INTO t_bios_device_type (name) VALUES ("ups");
INSERT INTO t_bios_device_type (name) VALUES ("epdu");
INSERT INTO t_bios_device_type (name) VALUES ("server");

/* insert dummy device to be used as a refferences for t_bios_client_info, which is not linked to any device */
SELECT @device_unclassified := id_device_type FROM t_bios_device_type WHERE name = 'not_classified';
INSERT INTO t_bios_discovered_device (id_discovered_device, name, id_device_type) VALUES
    (1, "DUMMY_DEVICE", @device_unclassified);
SELECT @dummy_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "DUMMY_DEVICE";

/* t_bios_asset_element_type */
INSERT INTO t_bios_asset_element_type (name) VALUES ("group");
INSERT INTO t_bios_asset_element_type (name) VALUES ("datacenter");
INSERT INTO t_bios_asset_element_type (name) VALUES ("room");
INSERT INTO t_bios_asset_element_type (name) VALUES ("row");
INSERT INTO t_bios_asset_element_type (name) VALUES ("rack");
INSERT INTO t_bios_asset_element_type (name) VALUES ("device");

/* t_bios_asset_device_type */
INSERT INTO t_bios_asset_device_type (name) VALUES ("ups");
INSERT INTO t_bios_asset_device_type (name) VALUES ("genset");
INSERT INTO t_bios_asset_device_type (name) VALUES ("epdu");
INSERT INTO t_bios_asset_device_type (name) VALUES ("pdu");
INSERT INTO t_bios_asset_device_type (name) VALUES ("server");
INSERT INTO t_bios_asset_device_type (name) VALUES ("feed");
INSERT INTO t_bios_asset_device_type (name) VALUES ("sts");
INSERT INTO t_bios_asset_device_type (name) VALUES ("switch");
INSERT INTO t_bios_asset_device_type (name) VALUES ("storage");
INSERT INTO t_bios_asset_device_type (name) VALUES ("vm");
INSERT INTO t_bios_asset_device_type (id_asset_device_type, name) VALUES (11, "N_A");

/* t_bios_asset_link_type */
INSERT INTO t_bios_asset_link_type (name) VALUES ("power chain");

/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
