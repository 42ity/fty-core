create database IF NOT EXISTS box_utf8 character set utf8 collate utf8_general_ci;

use box_utf8;

DROP TABLE t_bios_client_info;
DROP TABLE t_bios_client;

DROP TABLE t_bios_net_history;

DROP TABLE t_bios_discovered_ip;

DROP TABLE t_bios_discovered_device;

DROP TABLE t_bios_device_type;

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
    timestamp datetime NOT NULL,

    PRIMARY KEY(id_net_history)
);

CREATE TABLE t_bios_client(
    id_client TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
    name VARCHAR(25),

    PRIMARY KEY(id_client),

    UNIQUE INDEX `UI_t_bios_client_name` (`name` ASC)
);

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

insert into t_bios_device_type (id_device_type, name) values (1, "not_classified");






insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);
insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"select_device",1);
insert into t_bios_discovered_device(id_discovered_device,name,id_device_type) values(NULL,"set_name",1);

insert into t_bios_client(id_client,name) values(NULL,"mymodule");
insert into t_bios_client(id_client,name) values(NULL,"admin");

insert into t_bios_device_type(id_device_type,name) values (NULL,"UPS");








drop view v_bios_device_type;
create view v_bios_device_type as select id_device_type id, name from t_bios_device_type;

drop view v_bios_discovered_device;
create view v_bios_discovered_device as select id_discovered_device id, name , id_device_type from t_bios_discovered_device;

drop view v_bios_client;
create view v_bios_client as select id_client id, name from t_bios_client;

drop view v_bios_client_info;
create view v_bios_client_info as select id_client_info id, id_discovered_device , ext , timestamp , id_client from t_bios_client_info;

drop view v_bios_discovered_ip;
create view v_bios_discovered_ip as select id_ip id, iph,ipl, id_discovered_device, ip, timestamp from t_bios_discovered_ip;

drop view v_bios_net_history;
create view v_bios_net_history as select id_net_history , ipl,iph, ip , mac,mask, command, timestamp from t_bios_net_history;











drop view v_bios_ip_last;
create view v_bios_ip_last as select max(timestamp) datum, id_discovered_device, iph, ipl, ip from v_bios_discovered_ip group by ipl, iph;

drop view v_bios_client_info_last;
create view v_bios_client_info_last as select max(timestamp) datum, ext, id_discovered_device, id_client from v_bios_client_info  group by id_discovered_device, id_client;



