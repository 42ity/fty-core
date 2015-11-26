use box_utf8;

SELECT @karol_lab_temp := id FROM t_bios_measurement_topic
WHERE device_id = (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'KAROL-LAB') AND
      topic LIKE "temperature.thermal_zone0@%";

SELECT @karol_lab_temp_15m := id FROM t_bios_measurement_topic
WHERE device_id = (SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'KAROL-LAB') AND
      topic LIKE "temperature.thermal_zone0_arithmetic_mean_15m@%";


/* Test S1 */
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-09 23:57:12", 50000, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 00:00:00", 50, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 00:14:59", 100, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 00:19:59", 10, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 00:25:41", 20, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 00:15:00", 77, 0, @karol_lab_temp_15m);

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 02:00:00", 50, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 02:14:59", 100, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 02:20:00", 10, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 02:25:41", 20, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 02:30:42", 20000, 0, @karol_lab_temp);

INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-09 03:57:12", 50000, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 04:00:00", 50, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 04:14:59", 100, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 04:19:59", 10, 0, @karol_lab_temp);
INSERT INTO t_bios_measurement (timestamp, value, scale, topic_id) VALUES ("2015-03-10 04:25:41", 20, 0, @karol_lab_temp);

/*
Expected results for S1:
start_ts=20150310000000Z&end_ts=20150310020000Z&type=arithmetic_mean&step=15m&source=temperature.thermal_zone0&element_id=28
0:15    77,0        <-- value stored in db, not computed. Which is shown by fact that it would be 75 if computed as is in third curl request
0:30    ~99.435

start_ts=20150310021459Z&end_ts=20150310023000Z&type=arithmetic_mean&step=15m&element_id=28&source=temperature.thermal_zone0
start_ts=20150310021500Z&end_ts=20150310023000Z&type=arithmetic_mean&step=15m&element_id=28&source=temperature.thermal_zone0
2:15:00    75,0
2:30:00    15,0

start_ts=20150310040000Z&end_ts=20150310043000Z&type=arithmetic_mean&step=15m&element_id=28&source=temperature.thermal_zone0
4:15    75,0        <-- this test is the same as the first curl but and shown, when computed it results in 75 !
4:30    ~99.435
*/

