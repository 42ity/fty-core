/* TODO: description  */
use box_utf8;

/* First initialize a clean database and load basic data */

/* Create new server device 'AVG-SRV' and nest it under 'RACK1-LAB' rack */
SELECT @rack := id_asset_element FROM t_bios_asset_element WHERE name = 'RACK1-LAB';
INSERT INTO t_bios_asset_element (name , id_type, id_subtype, id_parent, asset_tag) VALUES 
    ('AVG-SRV',
     (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name ='device'),
     (SELECT id_asset_device_type FROM t_bios_asset_device_type WHERE name = 'server'),
     @rack,
     "AVG-SRV-asset-tag");
SET @element_id = LAST_INSERT_ID();

INSERT INTO t_bios_discovered_device (name, id_device_type) VALUES
    ('AVG-SRV',
     (select id_device_type from t_bios_device_type where name = 'server'));

INSERT INTO t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES
    ((SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'AVG-SRV'),
     (SELECT id_asset_element FROM t_bios_asset_element WHERE name = 'AVG-SRV'));

/* Create topics for 'AVG-SRV', i.e. add some quantities measured on 'AVG-SRV': */
/*    temperature.thermal_zone0     C                                           */
/*    realpower.default             W                                           */
/*    rocket.fuelpressure           MPa                                         */
/*    <add another measurement>                                                 */
INSERT INTO t_bios_measurement_topic (device_id, units, topic)
    SELECT r.id_discovered_device,'C','temperature.thermal_zone0@AVG-SRV'
    FROM t_bios_asset_element AS e,t_bios_monitor_asset_relation AS r WHERE
    e.name = 'AVG-SRV' AND e.id_asset_element = r.id_asset_element;
SET @topic_temperature = LAST_INSERT_ID();

INSERT INTO t_bios_measurement_topic (device_id, units, topic)
    SELECT r.id_discovered_device,'W','realpower.default@AVG-SRV'
    FROM t_bios_asset_element AS e,t_bios_monitor_asset_relation AS r WHERE
    e.name = 'AVG-SRV' AND e.id_asset_element = r.id_asset_element;
SET @topic_power = LAST_INSERT_ID();

INSERT INTO t_bios_measurement_topic (device_id, units, topic)
    SELECT r.id_discovered_device,'MPa','rocket.fuelpressure@AVG-SRV'
    FROM t_bios_asset_element AS e,t_bios_monitor_asset_relation AS r WHERE
    e.name = 'AVG-SRV' AND e.id_asset_element = r.id_asset_element;
SET @topic_pressure = LAST_INSERT_ID();

/* Insert measurements for given topic - test cases */


