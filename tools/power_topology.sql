insert into t_bios_asset_device_type (id_asset_device_type , name) values (5, "mgenset");

insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5000, "serv2", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5001, "ePDUA", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5002, "ePDUB", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5003, "Upps1", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5004, "main2", 6 , NULL); 
insert into t_bios_asset_element ( id_asset_element , name   , id_type , id_parent ) values (5005, "GenSetA", 6 , NULL); 

insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6000, 5000, 3);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6001, 5001, 2);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6002, 5002, 2);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6003, 5003, 1);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6004, 5004, 4);
insert into t_bios_asset_device ( id_asset_device , id_asset_element , id_asset_device_type ) values (6005, 5005, 5);

insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6001 , 2,6000 , 1, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6002 , 4,6000 , 3, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6000 , 5, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6001 , 7, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6003 , NULL,6002 , 6, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6004 , 8,6003 , NULL, 1);
insert into t_bios_asset_link (id_link , id_asset_device_src , src_out , id_asset_device_dest , dest_in , id_asset_link_type ) values (NULL, 6005 , 9,6003 , NULL, 1);

insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4999, "power_inputgrp_test", 1, NULL);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5005);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5003);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5002);
insert into t_bios_asset_group_relation (id_asset_group_relation , id_asset_group, id_asset_element) values (NULL, 4999, 5000);



insert into t_bios_asset_element ( id_asset_element , name , id_type , id_parent ) values (4998, "power_inputgrp_empty", 1, NULL);
