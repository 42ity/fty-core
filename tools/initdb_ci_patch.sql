use box_utf8
alter table t_bios_asset_element modify column asset_tag CHAR(10) NOT NULL DEFAULT "1234567890";
drop index UI_t_bios_asset_element_ASSET_TAG on t_bios_asset_element;

