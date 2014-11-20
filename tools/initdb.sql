DROP DATABASE IF EXISTS box_utf8;
CREATE DATABASE IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

USE box_utf8;

DROP TABLE if exists t_bios_monitor_asset_relation;
drop table if exists t_bios_discovered_ip;
drop table if exists t_bios_net_history;
drop table if exists t_bios_client_info_measurements;
drop table if exists t_bios_measurement_subtypes;
drop table if exists t_bios_measurement_types;
drop table if exists t_bios_client_info;
drop table if exists t_bios_client;
drop table if exists t_bios_discovered_device;
drop table if exists t_bios_device_type;

CREATE TABLE t_bios_measurement_types(
    id               SMALLINT UNSIGNED  NOT NULL AUTO_INCREMENT,
    name             VARCHAR(25) NOT NULL,
    PRIMARY KEY(id)
);

CREATE TABLE t_bios_measurement_subtypes(
    id               SMALLINT UNSIGNED  NOT NULL,
    type_id          SMALLINT UNSIGNED  NOT NULL,
    name             VARCHAR(25) NOT NULL,
    scale            TINYINT NOT NULL,

    PRIMARY KEY(id, type_id),

    INDEX(id, type_id),

    FOREIGN KEY(type_id)
	REFERENCES t_bios_measurement_types(id)
        ON DELETE RESTRICT
);

CREATE TABLE t_bios_device_type(
    id_device_type      TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name                VARCHAR(25)      NOT NULL,

    PRIMARY KEY(id_device_type),

    UNIQUE INDEX `UI_t_bios_device_type_name` (`name` ASC)

)AUTO_INCREMENT = 2;

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
    ip                      char(19)            NOT NULL,
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
    id_client TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(25),

    PRIMARY KEY(id_client),

    UNIQUE INDEX `UI_t_bios_client_name` (`name` ASC)
) AUTO_INCREMENT = 2;

CREATE TABLE t_bios_client_info(
    id_client_info          BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,
    id_client               TINYINT UNSIGNED    NOT NULL,
    id_discovered_device    SMALLINT UNSIGNED,
    timestamp               datetime            NOT NULL,
    ext                     BLOB,

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

) AUTO_INCREMENT = 6;

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
  ip                    CHAR(19),
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

  CONSTRAINT FK_ASSETLINK_TYPEEE
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

CREATE TABLE t_bios_client_info_measurements(
    id_measurements         BIGINT UNSIGNED     NOT NULL AUTO_INCREMENT,
    id_client               TINYINT UNSIGNED    NOT NULL,
    id_discovered_device    SMALLINT UNSIGNED   NOT NULL,
    timestamp               datetime            NOT NULL,
    id_subkey               SMALLINT UNSIGNED   NOT NULL,
    id_key                  SMALLINT UNSIGNED   NOT NULL,
    value                   INT                 NOT NULL,

    PRIMARY KEY(id_measurements),

    INDEX(id_discovered_device),
    INDEX(id_subkey, id_key),
    INDEX(id_client),

    FOREIGN KEY (id_subkey, id_key)
        REFERENCES t_bios_measurement_subtypes(id, type_id)
        ON DELETE RESTRICT,
    
    FOREIGN KEY (id_discovered_device)
        REFERENCES t_bios_discovered_device(id_discovered_device)
        ON DELETE RESTRICT,

    FOREIGN KEY (id_client)
        REFERENCES t_bios_client(id_client)
        ON DELETE RESTRICT
) ENGINE=INNODB;

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

create view v_bios_client_info_measurements as select  id_measurements as id, id_client , id_discovered_device, timestamp , id_key  ,  id_subkey , value from t_bios_client_info_measurements;

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
    st.id , st.type_id, st.name, st.scale, t.name as typename
FROM
    v_bios_measurement_types t,
    t_bios_measurement_subtypes st
where
    st.type_id = t.id;

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

create view v_bios_asset_link as select * from t_bios_asset_link ;
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
        sk.scale
FROM    v_bios_client_info_measurements v
        INNER JOIN v_bios_measurements_lastdate grp 
              ON v.id_key = grp.id_key AND
                 v.id_subkey = grp.id_subkey AND
                 v.timestamp = grp.maxdate  AND
                 v.id_discovered_device = grp.id_discovered_device
        INNER JOIN t_bios_measurement_subtypes sk
                ON v.id_subkey = sk.id AND
		   v.id_key = sk.type_id;
