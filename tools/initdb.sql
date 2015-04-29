DROP DATABASE IF EXISTS box_utf8;
CREATE DATABASE IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

USE box_utf8;

SET GLOBAL time_zone='+00:00';

/* work around smart insert without duplicates*/
CREATE TABLE IF NOT EXISTS t_empty (id TINYINT);
INSERT INTO t_empty values (1);

DROP TABLE if exists t_bios_monitor_asset_relation;
drop table if exists t_bios_discovered_ip;
drop table if exists t_bios_net_history;
drop table if exists t_bios_measurement;
drop table if exists t_bios_measurement_topic;
drop table if exists t_bios_client_info;
drop table if exists t_bios_client;
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
    timestamp     DATETIME            NOT NULL,
    value         INTEGER             NOT NULL,
    scale         SMALLINT            NOT NULL,
    topic_id      INTEGER UNSIGNED    NOT NULL,

    PRIMARY KEY(id),

    INDEX(topic_id),
    INDEX(timestamp),

    FOREIGN KEY(topic_id)
        REFERENCES t_bios_measurement_topic(id)
        ON DELETE CASCADE
);

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
    
    FOREIGN KEY(id_device_type)
	REFERENCES t_bios_device_type(id_device_type)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_alert(
    id          INT UNSIGNED     NOT NULL AUTO_INCREMENT,
    rule_name   VARCHAR(50)      NOT NULL,
    date_from   DATETIME         NOT NULL,
    priority    TINYINT UNSIGNED NOT NULL,
    state       TINYINT UNSIGNED NOT NULL,
    description VARCHAR(255),
    date_till    DATETIME,
    notification TINYINT         NOT NULL DEFAULT 0,

    PRIMARY KEY(id),

    INDEX(id),

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

CREATE TABLE t_bios_discovered_ip(
    id_ip                   INT UNSIGNED        NOT NULL  AUTO_INCREMENT,
    id_discovered_device    SMALLINT UNSIGNED,
    timestamp               datetime            NOT NULL,
    ip                      char(45)            NOT NULL,
    PRIMARY KEY(id_ip),

    INDEX(id_discovered_device),

    FOREIGN KEY (id_discovered_device)
        REFERENCES t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_net_history(
    id_net_history  INT UNSIGNED        NOT NULL AUTO_INCREMENT,
    command         CHAR(1)             NOT NULL,
    mac             CHAR(17),
    mask            TINYINT UNSIGNED    NOT NULL,
    ip              CHAR(19)            NOT NULL,
    name            VARCHAR(50),
    timestamp       datetime            NOT NULL,

    PRIMARY KEY(id_net_history)
);

CREATE TABLE t_bios_client(
    id_client   TINYINT UNSIGNED    NOT NULL AUTO_INCREMENT,
    name        VARCHAR(50)         NOT NULL,

    PRIMARY KEY(id_client),

    UNIQUE INDEX `UI_t_bios_client_name` (`name` ASC)
) AUTO_INCREMENT = 1;

CREATE TABLE t_bios_client_info(
    id_client_info          BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,
    id_client               TINYINT UNSIGNED    NOT NULL,
    id_discovered_device    SMALLINT UNSIGNED   NOT NULL,
    timestamp               datetime            NOT NULL,
    ext                     BLOB                NOT NULL,

    PRIMARY KEY(id_client_info),

    INDEX(id_discovered_device),
    INDEX(id_client),

    FOREIGN KEY (id_discovered_device)
        REFERENCEs t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT,

    FOREIGN KEY (id_client)
        REFERENCES t_bios_client(id_client)
        ON DELETE RESTRICT
);

DROP TABLE if exists t_bios_asset_link;
DROP TABLE if exists t_bios_asset_link_type;
DROP TABLE if exists t_bios_asset_device;
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

CREATE TABLE t_bios_asset_element (
  id_asset_element  INT UNSIGNED        NOT NULL AUTO_INCREMENT,
  name              VARCHAR(50)         NOT NULL,
  id_type           TINYINT UNSIGNED    NOT NULL,
  id_parent         INT UNSIGNED,
  status            char(9)             NOT NULL DEFAULT "noactive",
  priority          TINYINT             NOT NULL DEFAULT 5,
  business_crit     TINYINT             NOT NULL DEFAULT 0,

  PRIMARY KEY (id_asset_element),

  INDEX FK_ASSETELEMENT_ELEMENTTYPE_idx (id_type   ASC),
  INDEX FK_ASSETELEMENT_PARENTID_idx    (id_parent ASC),

  CONSTRAINT FK_ASSETELEMENT_ELEMENTTYPE
    FOREIGN KEY (id_type)
    REFERENCES t_bios_asset_element_type (id_asset_element_type)
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


CREATE TABLE t_bios_asset_device_type(
  id_asset_device_type TINYINT UNSIGNED   NOT NULL AUTO_INCREMENT,
  name                 VARCHAR(50)        NOT NULL,
  
  PRIMARY KEY (id_asset_device_type),
  UNIQUE INDEX `UI_t_bios_asset_device_type` (`name` ASC)

);


CREATE TABLE t_bios_asset_device (
  id_asset_device       INT UNSIGNED     NOT NULL AUTO_INCREMENT,
  id_asset_element      INT UNSIGNED     NOT NULL,
  id_asset_device_type  TINYINT UNSIGNED NOT NULL,

  PRIMARY KEY (id_asset_device),

  INDEX FK_ASSETDEVICE_ELEMENT_idx    (id_asset_device_type   ASC),
  INDEX FK_ASSETDEVICE_DEVICETYPE_idx (id_asset_device_type   ASC),

  CONSTRAINT FK_ASSETDEVICE_ELEMENT
    FOREIGN KEY (id_asset_element)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETDEVICE_DEVICETYPE
    FOREIGN KEY (id_asset_device_type)
    REFERENCES t_bios_asset_device_type (id_asset_device_type)
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
    REFERENCES t_bios_asset_device(id_asset_device)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETLINK_DEST
    FOREIGN KEY (id_asset_device_dest)
    REFERENCES t_bios_asset_device(id_asset_device)
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
drop view if exists v_bios_client;
drop view if exists v_bios_client_info;
drop view if exists v_bios_discovered_ip;
drop view if exists v_bios_net_history;

create view v_bios_device_type as select id_device_type id, name from t_bios_device_type;

create view v_bios_discovered_device as select id_discovered_device id, name , id_device_type from t_bios_discovered_device;

create view v_bios_client as select id_client id, name from t_bios_client;

create view v_bios_client_info as select id_client_info id, id_discovered_device , ext , timestamp , id_client from t_bios_client_info;

create view v_bios_discovered_ip as select id_ip id, id_discovered_device, ip, timestamp from t_bios_discovered_ip;

create view v_bios_net_history as select id_net_history id, ip , mac,mask, command, timestamp,name  from t_bios_net_history;

drop view if exists v_bios_ip_last;
drop view if exists v_bios_client_info_last;
drop view if exists v_bios_info_lastdate;

create view v_bios_ip_last as select max(timestamp) datum, id_discovered_device,  ip,id from v_bios_discovered_ip group by ip;

create view v_bios_info_lastdate as SELECT max(p.timestamp) maxdate, p.id_discovered_device, p.id_client FROM v_bios_client_info p  GROUP BY p.id_discovered_device, p.id_client;

create view v_bios_client_info_last as
SELECT  v.id,
        v.id_discovered_device,
        v.id_client,
        v.ext,
        v.timestamp
FROM    v_bios_client_info v
        INNER JOIN v_bios_info_lastdate grp 
              ON v.id_client = grp.id_client AND
                 v.timestamp = grp.maxdate  AND
                 v.id_discovered_device = grp.id_discovered_device;

DROP view if exists v_bios_asset_device;
DROP view if exists v_bios_asset_device_type;
DROP view if exists v_bios_asset_ext_attributes;
DROP view if exists v_bios_asset_group_relation;
DROP view if exists v_bios_asset_element;
DROP view if exists v_bios_asset_element_type;
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


CREATE VIEW v_bios_asset_device AS
    SELECT  v1.id_asset_device,
            v1.id_asset_element,
            v1.id_asset_device_type,
            v2.name
    FROM t_bios_asset_device v1
        LEFT JOIN t_bios_asset_device_type v2
        ON (v1.id_asset_device_type = v2.id_asset_device_type);

CREATE VIEW v_bios_asset_link AS
    SELECT  v1.id_link,
            v1.id_asset_device_src,
            v1.src_out,
            v1.id_asset_device_dest,
            v1.dest_in,
            v1.id_asset_link_type,
            v2.id_asset_element id_asset_element_src,
            v3.id_asset_element id_asset_element_dest
    FROM t_bios_asset_link v1
        LEFT JOIN v_bios_asset_device v2
        ON(v1.id_asset_device_src  = v2.id_asset_device)
        LEFT JOIN v_bios_asset_device v3
        ON(v1.id_asset_device_dest = v3.id_asset_device);

CREATE VIEW v_bios_asset_link_topology AS
    SELECT  v1.src_out,
            v1.dest_in,
            v1.id_asset_link_type,
            v2.id_asset_element AS id_asset_element_src,
            v4.name AS src_name,
            v3.id_asset_element AS id_asset_element_dest,
            v5.name AS dest_name,
            v6.name AS src_type_name,
            v7.name AS dest_type_name,
            v6.id_asset_device_type AS src_type_id,
            v7.id_asset_device_type AS dest_type_id
    FROM t_bios_asset_link v1
        LEFT JOIN t_bios_asset_device v2
        ON (v1.id_asset_device_src  = v2.id_asset_device)
        LEFT JOIN v_bios_asset_device v3
        ON (v1.id_asset_device_dest = v3.id_asset_device)
        LEFT JOIN t_bios_asset_element v4
        ON (v4.id_asset_element = v2.id_asset_element)
        LEFT JOIN t_bios_asset_element v5
        ON (v5.id_asset_element = v3.id_asset_element)
        LEFT JOIN t_bios_asset_device_type v6
        ON (v2.id_asset_device_type = v6.id_asset_device_type)
        LEFT JOIN t_bios_asset_device_type v7
        ON (v3.id_asset_device_type = v7.id_asset_device_type);

create view v_bios_asset_device_type as select id_asset_device_type as id, name from t_bios_asset_device_type ;
create view v_bios_asset_ext_attributes as select * from t_bios_asset_ext_attributes ;
create view v_bios_asset_group_relation as select * from t_bios_asset_group_relation ;
create view v_bios_asset_element as select v1.id_asset_element as id, v1.name, v1.id_type, v1.id_parent, v2.id_type as id_parent_type, v1.business_crit, v1.status, v1.priority from t_bios_asset_element v1 LEFT JOIN  t_bios_asset_element v2 on (v1.id_parent = v2.id_asset_element) ;
create view v_bios_asset_element_type as select id_asset_element_type as id, name from t_bios_asset_element_type ;
create view v_bios_monitor_asset_relation as select * from t_bios_monitor_asset_relation;

CREATE VIEW v_bios_asset_element_super_parent AS 
SELECT v1.id_asset_element, 
       v1.name , 
       v5.name AS type_name,
       v5.id_asset_device_type,
       v1.id_parent AS id_parent1,
       v2.id_parent AS id_parent2,
       v3.id_parent AS id_parent3,
       v4.id_parent AS id_parent4 
FROM t_bios_asset_element v1 
     LEFT JOIN t_bios_asset_element v2 
        ON (v1.id_parent = v2.id_asset_element) 
     LEFT JOIN t_bios_asset_element v3 
        ON (v2.id_parent = v3.id_asset_element) 
     LEFT JOIN t_bios_asset_element v4 
        ON (v3.id_parent=v4.id_asset_element) 
     INNER JOIN v_bios_asset_device v5 
        ON (v5.id_asset_element = v1.id_asset_element);

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

CREATE VIEW v_bios_measurement_lastdate AS
SELECT max(p.timestamp) maxdate,
       p.device_id,
       p.topic_id
FROM v_bios_measurement p
GROUP BY p.topic_id, p.device_id;

CREATE VIEW v_bios_measurement_last AS
SELECT  v.id,
        v.device_id,
        v.timestamp,
        v.topic,
        v.topic_id,
        v.value,
        v.scale
FROM       v_bios_measurement v
INNER JOIN v_bios_measurement_lastdate grp 
     ON v.timestamp = grp.maxdate  AND
        v.topic_id = grp.topic_id;

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

/* t_bios_client */
INSERT INTO t_bios_client (name) VALUES ("nmap");
INSERT INTO t_bios_client (name) VALUES ("mymodule");
INSERT INTO t_bios_client (name) VALUES ("admin");
INSERT INTO t_bios_client (name) VALUES ("NUT");
INSERT INTO t_bios_client (name) VALUES ("ui_properties");

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
INSERT INTO t_bios_asset_device_type (name) VALUES ("main");
INSERT INTO t_bios_asset_device_type (name) VALUES ("sts");

/* t_bios_asset_link_type */
INSERT INTO t_bios_asset_link_type (name) VALUES ("power chain");

/* ui/properties are somewhat special */
SELECT @client_ui_properties := id_client FROM t_bios_client WHERE name = 'ui_properties';
INSERT INTO t_bios_client_info (id_client, id_discovered_device, timestamp, ext) VALUES (@client_ui_properties, @dummy_device, NOW(), "{}");
