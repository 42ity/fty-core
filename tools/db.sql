create database IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

use box_utf8;

drop table if exists t_bios_discovered_ip;
drop table if exists t_bios_net_history;
drop table if exists t_bios_client_info;
drop table if exists t_bios_client;
drop table if exists t_bios_discovered_device;
drop table if exists t_bios_device_type;

CREATE TABLE t_bios_device_type(
    id_device_type TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(25) NOT NULL,
    PRIMARY KEY(id_device_type),
    UNIQUE INDEX `UI_t_bios_device_type_name` (`name` ASC)
)AUTO_INCREMENT = 2;

CREATE TABLE t_bios_discovered_device(
    id_discovered_device SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(25),
    id_device_type TINYINT UNSIGNED NOT NULL,

    PRIMARY KEY(id_discovered_device),
    
    INDEX(id_device_type),
    
    FOREIGN KEY(id_device_type)
	REFERENCES t_bios_device_type(id_device_type)
        ON DELETE CASCADE
);

CREATE TABLE t_bios_discovered_ip(
    id_ip INT UNSIGNED NOT NULL  AUTO_INCREMENT,
    ipl BIGINT UNSIGNED,
    iph BIGINT UNSIGNED,
    id_discovered_device SMALLINT UNSIGNED,
    timestamp datetime NOT NULL,
    ip char(19),
    PRIMARY KEY(id_ip),

    INDEX(id_discovered_device),

    FOREIGN KEY (id_discovered_device)
        REFERENCES t_bios_discovered_device(id_discovered_device)
        ON DELETE CASCADE
);

CREATE TABLE t_bios_net_history(
    id_net_history INT UNSIGNED NOT NULL AUTO_INCREMENT,
    command CHAR(1),
    mac BIGINT UNSIGNED,
    mask TINYINT UNSIGNED,
    ipl BIGINT UNSIGNED,
    iph BIGINT UNSIGNED,
    ip CHAR(19),
    name VARCHAR(25),
    timestamp datetime NOT NULL,

    PRIMARY KEY(id_net_history)
);

CREATE TABLE t_bios_client(
    id_client TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(25),

    PRIMARY KEY(id_client),

    UNIQUE INDEX `UI_t_bios_client_name` (`name` ASC)
) AUTO_INCREMENT = 2;

CREATE TABLE t_bios_client_info(
    id_client_info BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    id_client TINYINT UNSIGNED NOT NULL,
    id_discovered_device SMALLINT UNSIGNED,
    timestamp datetime NOT NULL,
    ext BLOB,

    PRIMARY KEY(id_client_info),

    INDEX(id_discovered_device),
    INDEX(id_client),

    FOREIGN KEY (id_discovered_device)
        REFERENCEs t_bios_discovered_device(id_discovered_device)
        ON DELETE CASCADE,

    FOREIGN KEY (id_client)
        REFERENCES t_bios_client(id_client)
        ON DELETE CASCADE
);

DROP TABLE if exists t_bios_asset_power_topology;
DROP TABLE if exists t_bios_asset_device;
DROP TABLE if exists t_bios_asset_device_type;
DROP TABLE if exists t_bios_asset_ext_attributes;
DROP TABLE if exists t_bios_asset_location_topology;
DROP TABLE if exists t_bios_asset_group_relation;
DROP TABLE if exists t_bios_asset_group;
DROP TABLE if exists t_bios_asset_group_type;
DROP TABLE if exists t_bios_asset_element;
DROP TABLE if exists t_bios_asset_element_type;

CREATE TABLE t_bios_asset_element_type (
  id_asset_element_type TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
  name                  VARCHAR(25)      NOT NULL,
  
  PRIMARY KEY (id_asset_element_type)
) AUTO_INCREMENT = 6;

CREATE TABLE t_bios_asset_element (
  id_asset_element  INT UNSIGNED NOT NULL AUTO_INCREMENT,
  name              VARCHAR(25),
  id_type           TINYINT UNSIGNED,
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

CREATE TABLE t_bios_asset_group_type(
  id_asset_group_type INT UNSIGNED NOT NULL AUTO_INCREMENT,
  name                VARCHAR(25)  NOT NULL,

  PRIMARY KEY (id_asset_group_type)
);

CREATE TABLE t_bios_asset_group(
  id_asset_group      INT UNSIGNED NOT NULL AUTO_INCREMENT,
  name                VARCHAR(25)  NOT NULL,
  id_asset_group_type INT UNSIGNED NOT NULL,

  PRIMARY KEY (id_asset_group),

  INDEX FK_ASSETGROUP_GROUPTYPE_idx (id_asset_group_type ASC),

  CONSTRAINT FK_ASSETGROUP_GROUPTYPE
    FOREIGN KEY (id_asset_group_type)
    REFERENCES t_bios_asset_group_type (id_asset_group_type)
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
    REFERENCES t_bios_asset_group (id_asset_group)
    ON DELETE RESTRICT
);


CREATE TABLE t_bios_asset_device_type(
  id_asset_device_type INT UNSIGNED   NOT NULL AUTO_INCREMENT,
  name                 VARCHAR(25)    NOT NULL,
  
  PRIMARY KEY (id_asset_device_type)
);

CREATE TABLE t_bios_asset_device (
  id_asset_device       INT UNSIGNED NOT NULL AUTO_INCREMENT,
  id_asset_element      INT UNSIGNED NOT NULL,
  hostname              VARCHAR(25),
  full_hostname         VARCHAR(45),
  ip                    VARCHAR(25),
  mac                   BIGINT,
  id_asset_device_type  INT UNSIGNED NOT NULL,

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

CREATE TABLE t_bios_asset_power_topology (
  id_power_topology     INT UNSIGNED    NOT NULL AUTO_INCREMENT,
  id_asset_device_src   INT UNSIGNED    NOT NULL,
  src_inlet             TINYINT,
  id_asset_device_dist  INT UNSIGNED    NOT NULL,
  dist_outlet           TINYINT,
  
  PRIMARY KEY (id_power_topology),
  
  INDEX FK_ASSETPOWERTOPOLOGY_SRC_idx  (id_asset_device_src    ASC),
  INDEX FK_ASSETPOWERTOPOLOGY_DIST_idx (id_asset_device_dist   ASC),
  
  CONSTRAINT FK_ASSETPOWERTOPOLOGY_SRC 
    FOREIGN KEY (id_asset_device_src)
    REFERENCES t_bios_asset_device(id_asset_device)
    ON DELETE RESTRICT,

  CONSTRAINT FK_ASSETPOWERTOPOLOGY_DIST_
    FOREIGN KEY (id_asset_device_dist)
    REFERENCES t_bios_asset_device(id_asset_device)
    ON DELETE RESTRICT
);

CREATE TABLE t_bios_asset_ext_attributes(
  id_asset_ext_attribute    INT UNSIGNED NOT NULL AUTO_INCREMENT,
  keytag                    VARCHAR(25),
  value                     VARCHAR(250),
  id_asset_element          INT UNSIGNED NOT NULL,
  
  PRIMARY KEY (id_asset_ext_attribute),
  
  INDEX FK_ASSETEXTATTR_ELEMENT_idx (id_asset_element ASC),
  
  CONSTRAINT FK_ASSETEXTATTR_ELEMENT
    FOREIGN KEY (id_asset_element)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);

CREATE TABLE t_bios_asset_location_topology (
  id_location_topology  INT UNSIGNED NOT NULL AUTO_INCREMENT,
  parent_id             INT UNSIGNED NOT NULL,
  child_id              INT UNSIGNED NOT NULL,

  PRIMARY KEY (id_location_topology),

  INDEX FK_TOPOLOGYLOCATION_CHILD_idx  (child_id  ASC),
  INDEX FK_TOPOLOGYLOCATION_PARENT_idx (parent_id ASC),

  CONSTRAINT FK_TOPOLOGYLOCATION_PARENT
    FOREIGN KEY (parent_id)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT,

  CONSTRAINT FK_TOPOLOGYLOCATION_CHILD
    FOREIGN KEY (child_id)
    REFERENCES t_bios_asset_element (id_asset_element)
    ON DELETE RESTRICT
);



insert into t_bios_device_type (id_device_type, name) values (1, "not_classified");
insert into t_bios_client (id_client, name) values (1, "nmap");

insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);
insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);

insert into t_bios_client(id_client,name) values(NULL,"mymodule");
insert into t_bios_client(id_client,name) values(NULL,"admin");

insert into t_bios_device_type(id_device_type,name) values (NULL,"UPS");

insert into t_bios_asset_element_type (id_asset_element_type, name) values (1, "datacenter");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (2, "room");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (3, "row");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (4, "rack");
insert into t_bios_asset_element_type (id_asset_element_type, name) values (5, "device");

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "DC1",1,NULL);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROOM1",2,1);

insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ROW1",3,2);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "RACK1",4,3);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "serv1",5,4);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "epdu",5,2);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "ups",5,2);
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent) values (NULL, "main",5,1);




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

create view v_bios_discovered_ip as select id_ip id, iph,ipl, id_discovered_device, ip, timestamp from t_bios_discovered_ip;

create view v_bios_net_history as select id_net_history id, ipl,iph, ip , mac,mask, command, timestamp,name  from t_bios_net_history;

drop view if exists v_bios_ip_last;
drop view if exists v_bios_client_info_last;

create view v_bios_ip_last as select max(timestamp) datum, id_discovered_device, iph, ipl, ip,id from v_bios_discovered_ip group by ipl, iph;

create view v_bios_client_info_last as select max(timestamp) datum, ext, id_discovered_device, id_client,id from v_bios_client_info  group by id_discovered_device, id_client;



DROP view if exists v_bios_asset_power_topology;
DROP view if exists v_bios_asset_device;
DROP view if exists v_bios_asset_device_type;
DROP view if exists v_bios_asset_ext_attributes;
DROP view if exists v_bios_asset_location_topology;
DROP view if exists v_bios_asset_group_relation;
DROP view if exists v_bios_asset_group;
DROP view if exists v_bios_asset_group_type;
DROP view if exists v_bios_asset_element;
DROP view if exists v_bios_asset_element_type;

create view v_bios_asset_power_topology as select * from t_bios_asset_power_topology ;
create view v_bios_asset_device as select * from t_bios_asset_device ;
create view v_bios_asset_device_type as select * from t_bios_asset_device_type ;
create view v_bios_asset_ext_attributes as select * from t_bios_asset_ext_attributes ;
create view v_bios_asset_location_topology as select * from t_bios_asset_location_topology ;
create view v_bios_asset_group_relation as select * from t_bios_asset_group_relation ;
create view v_bios_asset_group as select * from t_bios_asset_group ;
create view v_bios_asset_group_type as select * from t_bios_asset_group_type ;
create view v_bios_asset_element as select v1.id_asset_element as id, v1.name, v1.id_type, v1.id_parent, v2.id_type as id_parent_type from t_bios_asset_element v1 LEFT JOIN  t_bios_asset_element v2 on (v1.id_parent = v2.id_asset_element) ;
create view v_bios_asset_element_type as select * from t_bios_asset_element_type ;

