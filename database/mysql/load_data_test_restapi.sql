use box_utf8;

/* constants */

/* t_bios_device_type */
select @device_unclassified := id_device_type from t_bios_device_type where name = 'not_classified';

/* testing device */
SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "select_device";
SELECT @monitor_asset_measure := id_discovered_device FROM t_bios_discovered_device WHERE name = "monitor_asset_measure";

/* data */
insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet.2@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet_2 := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet.2@monitor_asset_measure";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet.1@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet_1 := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet.1@monitor_asset_measure";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @monitor_asset_measure, "W", "realpower.outlet@monitor_asset_measure" );
SELECT @test_topic_m_a_m_realpower_outlet  := id FROM t_bios_measurement_topic WHERE topic = "realpower.outlet@monitor_asset_measure";


insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "", "status.ups@select_device" );
SELECT @test_topic_s_d_status_ups := id FROM t_bios_measurement_topic WHERE topic = "status.ups@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "%", "charge.battery@select_device" );
SELECT @test_topic_s_d_charge_battery := id FROM t_bios_measurement_topic WHERE topic = "charge.battery@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "s", "runtime.battery@select_device" );
SELECT @test_topic_s_d_runtime_battery := id FROM t_bios_measurement_topic WHERE topic = "runtime.battery@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "%", "load.default@select_device" );
SELECT @test_topic_s_d_load_default := id FROM t_bios_measurement_topic WHERE topic = "load.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "C", "temperature.default@select_device" );
SELECT @test_topic_s_d_temperature_default := id FROM t_bios_measurement_topic WHERE topic = "temperature.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "W", "realpower.default@select_device" );
SELECT @test_topic_s_d_realpower_default := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "A", "current.output.L1@select_device" );
SELECT @test_topic_s_d_current_output_L1 := id FROM t_bios_measurement_topic WHERE topic = "current.output.L1@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "A", "current.output@select_device" );
SELECT @test_topic_s_d_current_output := id FROM t_bios_measurement_topic WHERE topic = "current.output@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "V", "voltage.output.L1-N@select_device" );
SELECT @test_topic_s_d_voltage_otput_L1_N := id FROM t_bios_measurement_topic WHERE topic = "voltage.output.L1-N@select_device";

insert into t_bios_measurement_topic (id, device_id, units, topic) values (NULL, @select_device, "V", "voltage.output@select_device" );
SELECT @test_topic_s_d_voltage_output := id FROM t_bios_measurement_topic WHERE topic = "voltage.output@select_device";


insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 50, 0, @test_topic_m_a_m_realpower_outlet_2 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 2405, -1, @test_topic_m_a_m_realpower_outlet_1 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 2405, -1, @test_topic_m_a_m_realpower_outlet );


insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 2, 0, @test_topic_s_d_status_ups );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 9310, -2, @test_topic_s_d_charge_battery );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 3600, 0, @test_topic_s_d_runtime_battery );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 17, -1, @test_topic_s_d_load_default );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 56, -1, @test_topic_s_d_temperature_default );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 1000, -4, @test_topic_s_d_realpower_default );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 12, -1, @test_topic_s_d_current_output_L1 );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 31, -1, @test_topic_s_d_current_output );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 10, -2, @test_topic_s_d_voltage_otput_L1_N );

insert into t_bios_measurement 
    (id, timestamp, value, scale, topic_id)
values 
    (NULL, NOW(), 3, -1, @test_topic_s_d_voltage_output );

