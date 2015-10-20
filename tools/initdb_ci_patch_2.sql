use box_utf8
alter table t_bios_asset_element add UNIQUE INDEX `UI_t_bios_asset_element_ASSET_TAG` (`asset_tag`  ASC);
