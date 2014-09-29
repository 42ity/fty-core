DROP TABLE v_bios_client_info;
DROP TABLE v_bios_client;
DROP TABLE v_bios_net_history;
DROP TABLE v_bios_discovered_ip;
DROP TABLE v_bios_discovered_device;
DROP TABLE v_bios_device_type;

CREATE TABLE v_bios_device_type(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(25)
);

CREATE TABLE v_bios_discovered_device(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(25),
    id_device_type INTEGER,

    FOREIGN KEY(id_device_type)
	REFERENCES v_bios_device_type(id_device_type)
        ON DELETE CASCADE
);

CREATE TABLE v_bios_discovered_ip(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ipl INTEGER UNSIGNED,
    iph INTEGER UNSIGNED,
    id_discovered_device INTEGER UNSIGNED,
    timestamp datetime NOT NULL,
    ip char(19),

    FOREIGN KEY (id_discovered_device)
        REFERENCES v_bios_discovered_device(id_discovered_device)
        ON DELETE CASCADE
);

CREATE TABLE v_bios_net_history(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    command CHAR(1),
    mac INTEGER UNSIGNED,
    mask INTEGER UNSIGNED,
    ipl INTEGER UNSIGNED,
    iph INTEGER UNSIGNED,
    ip CHAR(19),
    timestamp datetime NOT NULL
);

CREATE TABLE v_bios_client(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name VARCHAR(25)
);

CREATE TABLE v_bios_client_info(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    id_client INTEGER UNSIGNED NOT NULL,
    id_discovered_device INTEGER UNSIGNED,
    timestamp datetime NOT NULL,
    ext BLOB,

    FOREIGN KEY (id_discovered_device)
        REFERENCEs v_bios_discovered_device(id_discovered_device)
        ON DELETE CASCADE,

    FOREIGN KEY (id_client)
        REFERENCES v_bios_client(id_client)
        ON DELETE CASCADE
);

insert into v_bios_device_type (id, name) values (1, "not_classified");






insert into v_bios_discovered_device(id,name,id_device_type) values(NULL,"select_device",1);
insert into v_bios_discovered_device(id,name,id_device_type) values(NULL,"select_device",1);
insert into v_bios_discovered_device(id,name,id_device_type) values(NULL,"set_name",1);

insert into v_bios_client(id,name) values(NULL,"mymodule");
insert into v_bios_client(id,name) values(NULL,"admin");

insert into v_bios_device_type(id,name) values (NULL,"UPS");






drop view v_bios_ip_last;
create view v_bios_ip_last as select max(timestamp) datum, id_discovered_device, iph, ipl, ip,id from v_bios_discovered_ip group by ipl, iph;

drop view v_bios_client_info_last;
create view v_bios_client_info_last as select max(timestamp) datum, ext, id_discovered_device, id_client, id from v_bios_client_info  group by id_discovered_device, id_client;


