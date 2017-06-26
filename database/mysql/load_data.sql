use box_utf8;

/* Constants */

/* t_bios_asset_device_type */
select @asset_device_ups := id_asset_device_type from t_bios_asset_device_type where name = 'ups';
select @asset_device_epdu := id_asset_device_type from t_bios_asset_device_type where name = 'epdu';
select @asset_device_server := id_asset_device_type from t_bios_asset_device_type where name = 'server';
select @asset_device_feed := id_asset_device_type from t_bios_asset_device_type where name = 'feed';

/* t_bios_device_type */
select @device_unclassified := id_device_type from t_bios_device_type where name = 'not_classified';
select @device_ups := id_device_type from t_bios_device_type where name = 'ups';
select @device_epdu := id_device_type from t_bios_device_type where name = 'epdu';
select @device_pdu := id_device_type from t_bios_device_type where name = 'pdu';
select @device_server := id_device_type from t_bios_device_type where name = 'server';

/* t_bios_asset_element_type */
select @asset_element_group := id_asset_element_type from t_bios_asset_element_type where name ='group';
select @asset_element_datacenter := id_asset_element_type from t_bios_asset_element_type where name ='datacenter';
select @asset_element_room := id_asset_element_type from t_bios_asset_element_type where name ='room';
select @asset_element_row := id_asset_element_type from t_bios_asset_element_type where name ='row';
select @asset_element_rack := id_asset_element_type from t_bios_asset_element_type where name ='rack';
select @asset_element_device := id_asset_element_type from t_bios_asset_element_type where name ='device';

/* t_bios_asset_link_type; */
select @asset_link_powerchain := id_asset_link_type from t_bios_asset_link_type where name = 'power chain';

/* NOTE: subquery is current mysql Error 1093 workaround */
insert into t_bios_asset_element (id_asset_element, name, id_type, id_parent, asset_tag) values (1001, "DC1",     @asset_element_datacenter, NULL, "myasset1" );
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_parent, asset_tag)
values
(
    1002,
    "ROOM1",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC1') as subquery),
    "myasset2"
);
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_parent, asset_tag)
values
(
    1003,
    "ROW1",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
, "myasset3" );
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_parent, asset_tag)
values
(
    1004,
    "RACK1",
    @asset_element_rack,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROW1') as subquery),
    "myasset4"
);
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_subtype, id_parent, asset_tag)
values
(
    1005,
    "serv1",
    @asset_element_device,
    @asset_device_server,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'RACK1') as subquery)
, "myasset5" );
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_subtype, id_parent, asset_tag)
values
(
    1006,
    "epdu",
    @asset_element_device,
    @asset_device_epdu,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
, "myasset6");
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_subtype, id_parent, asset_tag)
values
(
    1007,
    "ups",
    @asset_element_device,
    @asset_device_ups,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM1') as subquery)
, "myasset7");
insert into t_bios_asset_element
    (id_asset_element, name, id_type, id_subtype, id_parent, asset_tag)
values
(
    1008,
    "feed",
    @asset_element_device,
    @asset_device_feed,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC1') as subquery)
, "myasset8");
insert into t_bios_asset_element (id_asset_element, name, id_type, id_parent, asset_tag)values (1009, "group1",  @asset_element_group,     NULL, "myasset9");
insert into t_bios_asset_element (name, id_type, id_parent, asset_tag)values ("DC2",     @asset_element_datacenter, NULL, "myasset10");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values
(
    "ROOM2",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
, "myasset11");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values
(
    "ROOM3",
    @asset_element_room,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
, "myasset12");
insert into t_bios_asset_element (id_asset_element, name, id_type, id_parent, asset_tag)values (1013, "ROOM4",   @asset_element_room,      NULL, "myasset13");
insert into t_bios_asset_element (id_asset_element, name, id_type, id_parent, asset_tag)values (1014, "ROW4",    @asset_element_row,       NULL, "myasset14");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values
(
    "ROW2",
    @asset_element_row, 
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM2') as subquery)
, "myasset15");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values
(
    "ROW3", 
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM3') as subquery)
, "myasset16");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values 
(
    "ROW5",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'ROOM3') as subquery)
, "myasset17");
insert into t_bios_asset_element
    (name, id_type, id_parent, asset_tag)
values
(
    "ROW6",
    @asset_element_row,
    (select id from (select id_asset_element as id from t_bios_asset_element where name = 'DC2') as subquery)
, "myasset18");

/* t_bios_asset_group_relation ('DC1', 'epdu') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'epdu')
);
/* t_bios_asset_group_relation ('DC1', 'ups') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'ups')
);
/* t_bios_asset_group_relation ('DC1', 'feed') */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from t_bios_asset_element where name = 'DC1'),
    (select id_asset_element from t_bios_asset_element where name = 'feed')
);

insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values
(
    (select id_asset_element from t_bios_asset_element where
        name = 'feed' and
        id_subtype = @asset_device_feed), /* 4 */
    (select id_asset_element from t_bios_asset_element where
        name = 'ups' and
        id_subtype = @asset_device_ups), /* 3 */
    @asset_link_powerchain /* 1 */,
    1,
    2
);

insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_element from t_bios_asset_element where
        name = 'ups' and
        id_subtype = @asset_device_ups), /* 3 */
    (select id_asset_element from t_bios_asset_element where
        name = 'epdu' and
        id_subtype = @asset_device_epdu),
    @asset_link_powerchain
);

insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values
(
    (select id_asset_element from t_bios_asset_element where
        name = 'epdu' and
        id_subtype = @asset_device_epdu),
    (select id_asset_element from t_bios_asset_element where
        name = 'serv1' and
        id_subtype = @asset_device_server),
    @asset_link_powerchain
);

insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "total_facility_load",
    "2",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "contact_name",
    "Mike Ekim",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "contact_email",
    "mike@nn.com",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "contact_phone",
    "555-2323",
    (select id_asset_element from t_bios_asset_element where name = 'DC2')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "description",
    "room is very cute",
    (select id_asset_element from t_bios_asset_element where name = 'ROOM4')
);
insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "flowers",
    "yes, in this room there are only flowers",
    (select id_asset_element from t_bios_asset_element where name = 'ROOM4')
);

insert into t_bios_asset_ext_attributes
    (keytag, value, id_asset_element)
values
(
    "type",
    "power_chain",
    (select id_asset_element from t_bios_asset_element where name = 'group1')
);

/* ************* */
/* MBT Rack data */
/* ************* */

/* DC-LAB */
insert into t_bios_asset_element (name , id_type, id_parent,status,priority, asset_tag)values ("DC-LAB", @asset_element_datacenter,  NULL, "active",1, "myasset19");
set @last_asset_element := LAST_INSERT_ID();
set @last_datacenter := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",    "EATON Montobonnot Datacenter (тест)", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("company",    "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("site",       "Montbonnot", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("country",    "France", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("address",    "110 Rue Blaise Pascal, 38330 Montbonnot-Saint-Martin", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "John Smith", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "john.smith@eaton.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);
/* ROOM1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent, status,priority, asset_tag) values (NULL, "ROOM1-LAB", @asset_element_room, @last_datacenter, "active",1,"myasset20");
set @last_asset_element := LAST_INSERT_ID();
set @last_room := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "Lab Room", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("floor", "0", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "John Smith", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "john.smith@eaton.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* RACK1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_parent, status,priority, asset_tag) values (2021, "RACK1-LAB", @asset_element_rack, @last_room, "active",1,"myasset21");
set @last_asset_element := 2021;
set @last_rack := @last_asset_element;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("model", "RESSPU4210KB 600mm x 1000mm - 42U Rack", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "Cooper",   @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",      "27",     @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("asset_tag",       "EATON123456",     @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("serial_no",       "21545212HGFV2",     @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "John Smith", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "john.smith@eaton.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* UPS1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2022, "UPS1-LAB", @asset_element_device, @asset_device_ups, @last_rack,"active",1, "myasset22");
set @last_asset_element := 2022;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS1 9PX 6kVA", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("end_warranty_date", "2018-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("warranty_expiration_date", "2020-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_warranty_expiration_date", "2019-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_maintenance_date", "2016-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "3",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "0",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("ip.1", "10.130.9.1", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1", "9PX-BiosDown", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("fqdn.1", "9PX-BiosDown.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Inspecteur Clouseau", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "clouseau@the-pink-panter.movie", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* UPS2-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2023, "UPS2-LAB", @asset_element_device, @asset_device_ups, @last_rack, "active",1,"myasset23");
set @last_asset_element := 2023;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "UPS2 9PX 6kVA", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("end_warranty_date", "2018-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("warranty_expiration_date", "2020-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_warranty_expiration_date", "2019-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_maintenance_date", "2016-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "3",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "3",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("ip.1", "10.130.9.1", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1", "9PX-BiosUp", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("fqdn.1", "9PX-BiosUp.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Inspecteur Clouseau", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "clouseau@the-pink-panter.movie", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* ePDU1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2024, "ePDU1-LAB", @asset_element_device, @asset_device_epdu, @last_rack, "active",1,"myasset24");
set @last_asset_element = 2024;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",    "ePDU1 eMAA10", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer",   "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("ip.1", "10.130.9.1", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1",       "MApdu-BiosLeft", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("fqdn.1",  "MApdu-BiosLeft.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",   "Hercule Poirot", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",  "hercule@poirot.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",  "+32-2-555-42-42", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_w_pos", "left",@last_asset_element);

/* ePDU2-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2025, "ePDU2-LAB", @asset_element_device, @asset_device_epdu, @last_rack,"active",1, "myasset25");
set @last_asset_element = 2025;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",    "ePDU2 eMAA10", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer",   "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1",       "MApdu-BiosRight", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("ip.1", "10.130.9.1", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("fqdn.1",  "MApdu-BiosRight.Bios.Labo.Kalif.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",   "Jean-Luc Picard", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",  "captain@ssenterprise.mil", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",  "555-4242", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_w_pos", "right",@last_asset_element);

/* SRV1-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2026, "SRV1-LAB",  @asset_element_device, @asset_device_server, @last_rack,"active",1, "myasset26");
set @last_asset_element = 2026;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV1 PRIMERGY RX100 G8",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "6",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "Fujitsu", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1",       "vesxi09", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Tintin", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "tintin+srv1-lab@tinint.net", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+32-2-555-42-42", @last_asset_element);

/* SRV2-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2027, "SRV2-LAB", @asset_element_device, @asset_device_server, @last_rack, "active",1,"myasset27");
set @last_asset_element = 2027;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV2 PRIMERGY RX100 G8", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "7",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "Fujitsu", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("hostname.1",       "vesxi10", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("fqdn.1",       "vesxi10.mbt.lab.etn.com", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Athos", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "athos@mousquetaires.defense.gouv.fr", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* KAROL-LAB */

insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, asset_tag) values (2028, "KAROL-LAB", @asset_element_device, @asset_device_server, @last_rack, "myasset28");
set @last_asset_element = 2028;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","Server for Karols average testing data", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("creator", "Vaporware", @last_asset_element);


/* FEED-LAB */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent, status,priority,asset_tag) values ("FEED-LAB", @asset_element_device, @asset_device_feed, @last_datacenter,"active",1, "myasset29");
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "FEED 3Ph 240V", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("phases.output",       "3",         @last_asset_element);

/* GROUP1-LAB */
insert into t_bios_asset_element (name , id_type, id_parent, status,priority,asset_tag) values ("GROUP1-LAB", @asset_element_group, @last_datacenter, "active",1,"myasset30");
set @last_asset_element = LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "input power chain", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("type",        "input_power",       @last_asset_element);




/* Create a group */
insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from `t_bios_asset_element` where name = 'GROUP1-LAB'),
    (select id_asset_element from `t_bios_asset_element` where name = 'FEED-LAB')
);


/* SRV3-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2031, "SRV3-LAB",  @asset_element_device, @asset_device_server, @last_rack, "active",1,"myasset50");
set @last_asset_element = 2031;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV3 DL320e G8",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "8",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "HP", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Porthos", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "porthos@mousquetaires.defense.gouv.fr", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* SRV4-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2032, "SRV4-LAB", @asset_element_device, @asset_device_server, @last_rack, "active",1,"myasset31");
set @last_asset_element = 2032;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV4 DL320e G8", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "9",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "HP", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Aramis", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "aramis@mousquetaires.defense.gouv.fr", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* SRV5-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2033, "SRV5-LAB",  @asset_element_device, @asset_device_server, @last_rack, "active",1,"myasset32");
set @last_asset_element = 2033;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description",        "SRV5 PowerEdge R320",  @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "11",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "Dell", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "D Artagnan", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "dartagnan@mousquetaires.defense.gouv.fr", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* SRV6-LAB */
insert into t_bios_asset_element (id_asset_element, name , id_type, id_subtype, id_parent, status,priority,asset_tag) values (2034, "SRV6-LAB", @asset_element_device, @asset_device_server, @last_rack, "active",1,"myasset33");
set @last_asset_element = 2034;
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description","SRV6 System x 3530 M4", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "1",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "13",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "IBM", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Prince Dakkar", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "nemo@nautilus.defense.gouv.fr", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

/* UPS3PH-LAB */
insert into t_bios_asset_element (name , id_type, id_subtype, id_parent, status,priority,asset_tag) values ("UPS3PH-LAB", @asset_element_device, @asset_device_ups, @last_datacenter, "active",1,"_myasset22");
set @last_asset_element := LAST_INSERT_ID();
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("description", "PXGX UPS + EATON Parallel BladeUPS", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("end_warranty_date", "2018-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("warranty_expiration_date", "2020-12-31", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_installation_date", "2014-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_warranty_expiration_date", "2019-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("battery_maintenance_date", "2016-11-12", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("u_size",  "40",        @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("location_u_pos",     "0",       @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("manufacturer", "EATON", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("ip.1", "10.130.36.13", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("full_hostname.1", "10.130.36.13", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_name",    "Stephane", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_email",    "stephane@the-pink-panter.movie", @last_asset_element);
insert into t_bios_asset_ext_attributes (keytag, value, id_asset_element) values ("contact_phone",    "+33 (0)4 42 42 42 42", @last_asset_element);

insert into t_bios_asset_group_relation
    (id_asset_group, id_asset_element)
values
(
    (select id_asset_element from `t_bios_asset_element` where name = 'GROUP1-LAB'),
    (select id_asset_element from `t_bios_asset_element` where name = 'UPS3PH-LAB')
);
/* Asset links */

/* link (FEED-LAB, UPS3PH-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'FEED-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS3PH-LAB'),
    @asset_link_powerchain
);

/* link (UPS3PH-LAB, UPS1-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'UPS3PH-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS1-LAB'),
    @asset_link_powerchain
);
/* link (UPS3PH-LAB, UPS2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'UPS3PH-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'UPS2-LAB'),
    @asset_link_powerchain
);
/* link (UPS1-LAB, ePDU2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'UPS1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    @asset_link_powerchain
);
/* link (UPS2-LAB, ePDU2-LAB, 'power chain') */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'UPS2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    @asset_link_powerchain
);
/* link (ePDU1-LAB, SRV1-LAB, 'power chain', B6, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    "13", 
    "1"
);
/* link (ePDU2-LAB, SRV1-LAB, 'power chain', B6, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV1-LAB'),
    @asset_link_powerchain,
    "13",
    "1"
);
/* link (ePDU1-LAB, SRV2-LAB, 'power chain', B5, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    "12",
    "1"
);
/* link (ePDU2-LAB, SRV2-LAB, 'power chain', B5, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV2-LAB'),
    @asset_link_powerchain,
    "12",
    "2"
);

/* link (ePDU1-LAB, SRV3-LAB, 'power chain', B2, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV3-LAB'),
    @asset_link_powerchain,
    "9",
    "1"
);
/* link (ePDU2-LAB, SRV3-LAB, 'power chain', B2, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV3-LAB'),
    @asset_link_powerchain,
    "9",
    "2"
);

/* link (ePDU1-LAB, SRV4-LAB, 'power chain', B1, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV4-LAB'),
    @asset_link_powerchain,
    "8",
    "1"
);
/* link (ePDU2-LAB, SRV4-LAB, 'power chain', B1, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV4-LAB'),
    @asset_link_powerchain,
    "8",
    "2"
);

/* link (ePDU1-LAB, SRV5-LAB, 'power chain', A7, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV5-LAB'),
    @asset_link_powerchain,
    "7",
    "1"
);
/* link (ePDU2-LAB, SRV5-LAB, 'power chain', A7, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV5-LAB'),
    @asset_link_powerchain,
    "7",
    "2"
);

/* link (ePDU1-LAB, SRV6-LAB, 'power chain', A5, 1) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU1-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV6-LAB'),
    @asset_link_powerchain,
    "5",
    "1"
);
/* link (ePDU2-LAB, SRV6-LAB, 'power chain', A5, 2) */
insert into t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in)
values 
(
    (select id_asset_element from t_bios_asset_element where name = 'ePDU2-LAB'),
    (select id_asset_element from t_bios_asset_element where name = 'SRV6-LAB'),
    @asset_link_powerchain,
    "5",
    "2"
);

