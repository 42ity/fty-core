use box_utf8
DROP PROCEDURE IF EXISTS insert_measurement;
DELIMITER $$
CREATE procedure insert_measurement(IN topic INT, IN day INT)
BEGIN
declare h INT ;
declare d INT ;
SET d=day;
labelday: WHILE d >= 0 DO
  SET h = 24;
  label24h: WHILE h > 0 DO
    INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES (DATE_ADD(CURRENT_DATE, INTERVAL -(d*24+h) HOUR) , 1000*d+10*h, -2, topic);
    SET h = h - 1;
  END WHILE label24h;
  SET d = d - 1;
END WHILE labelday;
END;$$
DELIMITER ;

SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "UPS1-LAB";
INSERT INTO t_bios_measurement_topic (device_id, units, topic) values (@select_device, "W","realpower.default@UPS1-LAB" );
SELECT @topic_id := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@UPS1-LAB";
CALL insert_measurement(@topic_id,30);

SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "UPS2-LAB";
INSERT INTO t_bios_measurement_topic (device_id, units, topic) values (@select_device, "W","realpower.default@UPS2-LAB" );
SELECT @topic_id := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@UPS2-LAB";
CALL insert_measurement(@topic_id,30);

SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "ePDU1-LAB";
INSERT INTO t_bios_measurement_topic (device_id, units, topic) values (@select_device, "W","realpower.default@ePDU1-LAB" );
SELECT @topic_id := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@ePDU1-LAB";
CALL insert_measurement(@topic_id,30);

SELECT @select_device := id_discovered_device FROM t_bios_discovered_device WHERE name = "ePDU2-LAB";
INSERT INTO t_bios_measurement_topic (device_id, units, topic) values (@select_device, "W","realpower.default@ePDU2-LAB" );
SELECT @topic_id := id FROM t_bios_measurement_topic WHERE topic = "realpower.default@ePDU2-LAB";
CALL insert_measurement(@topic_id,30);
