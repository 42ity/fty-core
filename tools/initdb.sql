DROP DATABASE IF EXISTS box_utf8;
CREATE DATABASE IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

USE box_utf8;

DROP TABLE if exists t_bios_monitor_asset_relation;
drop table if exists t_bios_discovered_ip;
drop table if exists t_bios_net_history;
drop table if exists t_bios_measurements;
drop table if exists t_bios_measurement_subtypes;
drop table if exists t_bios_measurement_types;
drop table if exists t_bios_client_info;
drop table if exists t_bios_client;
drop table if exists t_bios_discovered_device;
drop table if exists t_bios_device_type;

CREATE TABLE t_bios_measurement_types(
    id               SMALLINT UNSIGNED  NOT NULL AUTO_INCREMENT,
    name             VARCHAR(25) NOT NULL,
    unit             VARCHAR(10) NOT NULL,
    PRIMARY KEY(id)
);

CREATE TABLE t_bios_measurement_subtypes(
    id               SMALLINT UNSIGNED  NOT NULL,
    id_type          SMALLINT UNSIGNED  NOT NULL,
    name             VARCHAR(25) NOT NULL,
    scale            TINYINT NOT NULL,

    PRIMARY KEY(id, id_type),
    INDEX(id),
    INDEX(id_type),
    INDEX(id_type, name),

    FOREIGN KEY(id_type)
	REFERENCES t_bios_measurement_types(id)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_device_type(
    id_device_type      TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name                VARCHAR(25)      NOT NULL,

    PRIMARY KEY(id_device_type),

    UNIQUE INDEX `UI_t_bios_device_type_name` (`name` ASC)

) AUTO_INCREMENT = 1;

CREATE TABLE t_bios_discovered_device(
    id_discovered_device    SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name                    VARCHAR(25)       NOT NULL,
    id_device_type          TINYINT UNSIGNED  NOT NULL,

    PRIMARY KEY(id_discovered_device),
    
    INDEX(id_device_type),
    
    FOREIGN KEY(id_device_type)
	REFERENCES t_bios_device_type(id_device_type)
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
    name            VARCHAR(25),
    timestamp       datetime            NOT NULL,

    PRIMARY KEY(id_net_history)
);

CREATE TABLE t_bios_client(
    id_client   TINYINT UNSIGNED    NOT NULL AUTO_INCREMENT,
    name        VARCHAR(25)         NOT NULL,

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
  name                  VARCHAR(25)      NOT NULL,
  
  PRIMARY KEY (id_asset_element_type),
  
  UNIQUE INDEX `UI_t_bios_asset_element_type` (`name` ASC)

) AUTO_INCREMENT = 1;

CREATE TABLE t_bios_asset_element (
  id_asset_element  INT UNSIGNED        NOT NULL AUTO_INCREMENT,
  name              VARCHAR(25)         NOT NULL,
  id_type           TINYINT UNSIGNED    NOT NULL,
  id_parent         int UNSIGNED,

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
  name                 VARCHAR(25)        NOT NULL,
  
  PRIMARY KEY (id_asset_device_type),
  UNIQUE INDEX `UI_t_bios_asset_device_type` (`name` ASC)

);


CREATE TABLE t_bios_asset_device (
  id_asset_device       INT UNSIGNED     NOT NULL AUTO_INCREMENT,
  id_asset_element      INT UNSIGNED     NOT NULL,
  hostname              VARCHAR(25),
  full_hostname         VARCHAR(45),
  ip                    CHAR(45),
  mac                   CHAR(17),
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
  name                 VARCHAR(25)        NOT NULL,
  
  PRIMARY KEY (id_asset_link_type),
  UNIQUE INDEX `UI_t_bios_asset_link_type_name` (`name` ASC)

);

CREATE TABLE t_bios_asset_link (
  id_link               INT UNSIGNED        NOT NULL AUTO_INCREMENT,
  id_asset_device_src   INT UNSIGNED        NOT NULL,
  src_out               TINYINT,
  id_asset_device_dest  INT UNSIGNED        NOT NULL,
  dest_in               TINYINT,
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
  keytag                    VARCHAR(25)  NOT NULL,
  value                     VARCHAR(255) NOT NULL,
  id_asset_element          INT UNSIGNED NOT NULL,
  
  PRIMARY KEY (id_asset_ext_attribute),
  
  INDEX FK_ASSETEXTATTR_ELEMENT_idx (id_asset_element ASC),
  
  CONSTRAINT FK_ASSETEXTATTR_ELEMENT
    FOREIGN KEY (id_asset_element)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);

CREATE TABLE t_bios_measurements (
    id_measurements         BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,
    id_client               TINYINT UNSIGNED    NOT NULL,
    id_device               SMALLINT UNSIGNED   NOT NULL,
    timestamp               datetime            NOT NULL,
    id_subtype              SMALLINT UNSIGNED   NOT NULL,
    id_type                 SMALLINT UNSIGNED   NOT NULL,
    value                   BIGINT              NOT NULL,

    PRIMARY KEY(id_measurements),

    INDEX (id_device),
    INDEX (id_subtype, id_type),
    INDEX(id_client),

    FOREIGN KEY (id_subtype, id_type)
        REFERENCES t_bios_measurement_subtypes(id, id_type)
        ON DELETE RESTRICT,
    
    FOREIGN KEY (id_device)
        REFERENCES t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT,

    FOREIGN KEY (id_client)
        REFERENCES t_bios_client(id_client)
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
drop view if exists v_bios_client_info_measurements;


create view v_bios_device_type as select id_device_type id, name from t_bios_device_type;

create view v_bios_discovered_device as select id_discovered_device id, name , id_device_type from t_bios_discovered_device;

create view v_bios_client as select id_client id, name from t_bios_client;

create view v_bios_client_info as select id_client_info id, id_discovered_device , ext , timestamp , id_client from t_bios_client_info;

create view v_bios_discovered_ip as select id_ip id, id_discovered_device, ip, timestamp from t_bios_discovered_ip;

create view v_bios_net_history as select id_net_history id, ip , mac,mask, command, timestamp,name  from t_bios_net_history;

create view v_bios_client_info_measurements as select  id_measurements as id, id_client, id_device as id_discovered_device, timestamp, id_type as id_key, id_subtype as id_subkey, value from t_bios_measurements;

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
DROP view if exists v_bios_client_info_measurements_last;
DROP VIEW IF EXISTS v_bios_measurements_lastdate;
DROP VIEW IF EXISTS v_bios_monitor_asset_relation;
DROP VIEW IF EXISTS v_bios_measurement_subtypes;
DROP VIEW IF EXISTS v_bios_measurement_types;


create view v_bios_measurement_types as select * from t_bios_measurement_types ;

create view v_bios_measurement_subtypes as
SELECT
    st.id , st.id_type, st.name, st.scale, t.name as typename
FROM
    v_bios_measurement_types t,
    t_bios_measurement_subtypes st
where
    st.id_type = t.id;

CREATE VIEW v_bios_asset_device AS
    SELECT  v1.id_asset_device,
            v1.id_asset_element,
            v1.hostname,
            v1.full_hostname,
            v1.ip,
            v1.mac,
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

create view v_bios_asset_device_type as select * from t_bios_asset_device_type ;
create view v_bios_asset_ext_attributes as select * from t_bios_asset_ext_attributes ;
create view v_bios_asset_group_relation as select * from t_bios_asset_group_relation ;
create view v_bios_asset_element as select v1.id_asset_element as id, v1.name, v1.id_type, v1.id_parent, v2.id_type as id_parent_type from t_bios_asset_element v1 LEFT JOIN  t_bios_asset_element v2 on (v1.id_parent = v2.id_asset_element) ;
create view v_bios_asset_element_type as select * from t_bios_asset_element_type ;
create view v_bios_monitor_asset_relation as select * from t_bios_monitor_asset_relation;
create view v_bios_measurements_lastdate as SELECT p.id_key, max(p.timestamp) maxdate, p.id_subkey, p.id_discovered_device FROM v_bios_client_info_measurements p  GROUP BY p.id_key, p.id_subkey, p.id_discovered_device;

create view v_bios_client_info_measurements_last as
SELECT  v.id,
        v.id_discovered_device,
        v.id_key,
        v.id_subkey,
        v.value,
        v.timestamp,
        sk.scale,
        dd.name
FROM    v_bios_client_info_measurements v
        INNER JOIN v_bios_measurements_lastdate grp 
              ON v.id_key = grp.id_key AND
                 v.id_subkey = grp.id_subkey AND
                 v.timestamp = grp.maxdate  AND
                 v.id_discovered_device = grp.id_discovered_device
        INNER JOIN t_bios_measurement_subtypes sk
              ON v.id_subkey = sk.id AND
                 v.id_key = sk.id_type
        INNER JOIN v_bios_discovered_device dd
              ON v.id_discovered_device = dd.id
        ;

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

/* *************************************************************************** */
/* **********************          INSERTIONS          *********************** */
/* *************************************************************************** */

/* t_bios_measurement_types */
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (1, "voltage", "V");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (2, "current", "A");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (3, "realpower", "W");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (4, "temperature", "C");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (5, "load", "%");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (6, "charge", "%");
INSERT INTO t_bios_measurement_types (id, name, unit) VALUES (7, "status", "");


/* t_bios_measurement_subtypes */
/* voltage.* */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 1, "output", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (2, 1, "output.L1-N", -1); 
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (3, 1, "output.L2-N", -1); 
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (4, 1, "output.L3-N", -1);
/* current.* */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 2, "output", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (2, 2, "output.L1", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (3, 2, "output.L2", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (4, 2, "output.L3", -1);
/* realpower.* */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 3, "default", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (2, 3, "output.L1", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (3, 3, "output.L2", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (4, 3, "output.L3", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (5, 3, "outlet", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (6, 3, "outlet.1", -1);
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (7, 3, "outlet.2", -1);
/* temperature */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 4, "default", -1);
/* load */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 5, "default", -1);
/* charge */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 6, "battery", -1);
/* status */
INSERT INTO t_bios_measurement_subtypes (id, id_type, name, scale) VALUES (1, 7, "ups", 0);


/* t_bios_device_type */
INSERT INTO t_bios_device_type (name) VALUES ("not_classified");
INSERT INTO t_bios_device_type (name) VALUES ("ups");
INSERT INTO t_bios_device_type (name) VALUES ("epdu");
INSERT INTO t_bios_device_type (name) VALUES ("pdu");
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

/* t_bios_asset_link_type */
INSERT INTO t_bios_asset_link_type (name) VALUES ("power chain");

/* ui/properties are somewhat special */
SELECT @client_ui_properties := id_client FROM t_bios_client WHERE name = 'ui_properties';
INSERT INTO t_bios_client_info (id_client, id_discovered_device, timestamp, ext) VALUES (@client_ui_properties, @dummy_device, NOW(), "{}");
