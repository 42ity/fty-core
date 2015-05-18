use box_utf8;
/* Constants */

/* t_bios_asset_device_type */
SELECT @asset_device_ups := id_asset_device_type
    FROM t_bios_asset_device_type WHERE name = 'ups';
SELECT @asset_device_epdu := id_asset_device_type
    FROM t_bios_asset_device_type WHERE name = 'epdu';
SELECT @asset_device_main := id_asset_device_type
    FROM t_bios_asset_device_type WHERE name = 'main';

/* t_bios_asset_element_type */
SELECT @asset_element_datacenter := id_asset_element_type
    FROM t_bios_asset_element_type WHERE name = 'datacenter';
SELECT @asset_element_rack := id_asset_element_type
    FROM t_bios_asset_element_type WHERE name = 'rack';
SELECT @asset_element_device := id_asset_element_type
    FROM t_bios_asset_element_type WHERE name = 'device';

/* t_bios_asset_link_type */
SELECT @asset_link_powerchain := id_asset_link_type
    FROM t_bios_asset_link_type WHERE name = 'power chain';

/* DC1 */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ('DC1', @asset_element_datacenter, NULL);
SET @last_asset_element := LAST_INSERT_ID();
SET @last_datacenter := @last_asset_element; 

/* DC1 ups */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ('UPS1-DC1', @asset_element_device, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_ups);

/* DC1 main */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ('main-DC1', @asset_element_device, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_main);

/* RACK1 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent)
VALUES (NULL, 'RACK1', @asset_element_rack, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
SET @last_rack := @last_asset_element;
/* RACK1 ups */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ('UPS2-RACK1', @asset_element_device, @last_rack);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_ups);

/* RACK2 */
INSERT INTO t_bios_asset_element (id_asset_element, name, id_type, id_parent)
VALUES (NULL, 'RACK2', @asset_element_rack, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
SET @last_rack := @last_asset_element;
/* RACK2 epdu */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ('EPDU1-RACK2', @asset_element_device, @last_rack);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_epdu);

/* Asset links */

/* link (main-DC1, UPS1-DC1, 'power chain') */
INSERT INTO t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
VALUES
(
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
        ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'main-DC1'),
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
     ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS1-DC1'),
    @asset_link_powerchain
);

/* link (UPS1-DC1, EPDU1-RACK2, 'power chain') */
INSERT INTO t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
VALUES
(
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
        ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS1-DC1'),
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
     ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'EPDU1-RACK2'),
    @asset_link_powerchain
);

/* link (UPS1-DC1, UPS2-RACK1, 'power chain') */
INSERT INTO t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
VALUES
(
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
        ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS1-DC1'),
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
     ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS2-RACK1'),
    @asset_link_powerchain
);

/* main  unlockated*/
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ("main-unlockated", @asset_element_device, NULL);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_main);

/* DC2 */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ("DC2", @asset_element_datacenter, NULL);
SET @last_asset_element := LAST_INSERT_ID();
SET @last_datacenter := @last_asset_element; 

/* DC2 ups */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ("UPS3-DC2", @asset_element_device, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_ups);

/* DC2 epdu */
INSERT INTO t_bios_asset_element (name , id_type, id_parent)
VALUES ("EPDU2-DC2", @asset_element_device, @last_datacenter);
SET @last_asset_element := LAST_INSERT_ID();
INSERT INTO t_bios_asset_device  (id_asset_element, id_asset_device_type)
VALUES (@last_asset_element, @asset_device_epdu);


/* Asset links */

/* link (main-unlockated, UPS3-DC2, 'power chain') */
INSERT INTO t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
VALUES
(
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
        ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'main-unlockated'),
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
     ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS3-DC2'),
    @asset_link_powerchain
);

/* link (UPS1-DC1, EPDU2-DC2, 'power chain') */
INSERT INTO t_bios_asset_link
    (id_asset_device_src, id_asset_device_dest, id_asset_link_type)
VALUES
(
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
        ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'UPS1-DC1'),
    (SELECT id_asset_device
     FROM t_bios_asset_device AS t1
     INNER JOIN t_bios_asset_element AS t2
     ON t1.id_asset_element = t2.id_asset_element
     WHERE name = 'EPDU2-DC2'),
    @asset_link_powerchain
);
