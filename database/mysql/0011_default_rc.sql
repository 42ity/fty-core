/* For details on schema version support see the main initdb.sql */
SET @bios_db_schema_version = '201710020001' ;
SET @bios_db_schema_filename = '0011_default_rc.sql' ;

use box_utf8;


/* This should be the first action in the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('begin-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'begin-import' order by id desc limit 1;
COMMIT;

/* ONLY FOR TESTING of this SQL script: precreate datacenter(s) and/or some rack controller(s) in the assets:
INSERT INTO t_bios_asset_element (name, id_type, id_parent, status, priority, asset_tag) VALUES ('dc-1', (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "datacenter"), NULL, 'active', 1, NULL);
INSERT INTO t_bios_asset_element (name, id_type, id_subtype, id_parent, status, priority, asset_tag) VALUES ('rackcontroller-0', (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "device" LIMIT 1), (SELECT id_asset_device_type FROM t_bios_asset_device_type WHERE name = "rack controller" LIMIT 1), (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type IN (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "datacenter") LIMIT 1), 'active', 1, NULL);
*/

/* Slurp current system identification variables */
/* NOTE: Revise this bit if mysql/mariadb server and client (rack controller)
 * would be different systems - make sure then that the envvars are those of
 * the RC (might have to pre-set this in db-init script and then source the
 * SQL bits to apply to schema)!
 */
\! /bin/rm -f /tmp/0011_default_rc-fty-envvars.sql
\! /usr/share/bios/scripts/envvars-ExecStartPre.sh -O /tmp/0011_default_rc-fty-envvars.sql --sql
SOURCE /tmp/0011_default_rc-fty-envvars.sql ;
\! /bin/rm -f /tmp/0011_default_rc-fty-envvars.sql

/* Define an overridable manner of selecting a "myself" rack controller
 * to pick one from several available (based on known UUID, Serial Number,
 * hostname (fqdn or asset user-friendly name), or IP address). Fall back
 * to the rack controller with lowest database ID number which does not have
 * any of those matching fields populated (known mismatch is not-myself).
 * Assets where higher-priority attributes were defined and did not match
 * should not be considered and matched even if a lower-priority attribute
 * yields a hit (e.g. uuid known and different, but same current IP address).
 * Returns NULL if no match was found, causing creation of a new asset as
 * 'rackcontroller-0', with a chance that this would be not the only RC in DB.
 * If an override is needed - just make sure this FUNCTION select_RC_myself()
 * with content you want gets defined earlier. Note that this only gets
 * used if we do at all need to upgrade existing database contents. */

DROP FUNCTION IF EXISTS select_RC_myself_default ;
DELIMITER //
CREATE FUNCTION select_RC_myself_default()  RETURNS INT UNSIGNED
  BEGIN
      DECLARE myid INT UNSIGNED;
      DECLARE id_type_device TINYINT(3) UNSIGNED;
      DECLARE id_subtype_rc TINYINT(3) UNSIGNED;
      SET myid = NULL;
      SET id_type_device = (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "device" LIMIT 1);
      SET id_subtype_rc = (SELECT id_asset_device_type FROM t_bios_asset_device_type WHERE name = "rack controller" LIMIT 1);

      IF @ENV_HARDWARE_UUID IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "uuid" AND tea.value = @ENV_HARDWARE_UUID
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_HARDWARE_SERIAL_NUMBER IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "serial_no" AND tea.value = @ENV_HARDWARE_SERIAL_NUMBER
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("uuid"))
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_HOSTNAME IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "fqdn" AND tea.value = @ENV_HOSTNAME
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("serial_no", "uuid"))
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_KNOWNFQDNS IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "fqdn" AND FIND_IN_SET(tea.value, @ENV_KNOWNFQDNS)
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("serial_no", "uuid"))
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea3 WHERE
                tea3.id_asset_element = tea.id_asset_element AND
                tea3.keytag = "fqdn" AND tea3.value = @ENV_HOSTNAME)
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_HOSTNAME IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "name" AND tea.value = @ENV_HOSTNAME
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("fqdn", "serial_no", "uuid"))
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_KNOWNFQDNS IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag = "name" AND FIND_IN_SET(tea.value, @ENV_KNOWNFQDNS)
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("fqdn", "serial_no", "uuid"))
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea3 WHERE
                tea3.id_asset_element = tea.id_asset_element AND
                tea3.keytag = "name" AND tea3.value = @ENV_HOSTNAME)
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      IF @ENV_IPADDRS IS NOT NULL THEN
        SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel, t_bios_asset_ext_attributes AS tea WHERE
            tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc AND
            tea.id_asset_element = tel.id_asset_element AND
            tea.keytag LIKE "ip.%" AND FIND_IN_SET(tea.value, @ENV_IPADDRS)
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea2 WHERE
                tea2.id_asset_element = tea.id_asset_element AND
                tea2.keytag IN ("fqdn", "serial_no", "uuid"))
            AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea3 WHERE
                tea3.id_asset_element = tea.id_asset_element AND
                tea3.keytag = "name" AND ( FIND_IN_SET(tea3.value, @ENV_KNOWNFQDNS) OR tea3.value = @ENV_HOSTNAME ) )
            ORDER BY tel.id_asset_element LIMIT 1);
        IF myid IS NOT NULL THEN
          RETURN myid;
        END IF;
      END IF;

      SET myid = (SELECT DISTINCT tel.id_asset_element FROM t_bios_asset_element AS tel WHERE
        tel.id_type = id_type_device AND tel.id_subtype = id_subtype_rc
        AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea WHERE
            tea.id_asset_element = tel.id_asset_element AND
            (tea.keytag LIKE "ip.%" OR tea.keytag IN ("fqdn", "serial_no", "uuid") ) )
        AND NOT EXISTS (SELECT * FROM t_bios_asset_ext_attributes AS tea3 WHERE
            tea3.id_asset_element = tel.id_asset_element AND
            tea3.keytag = "name" AND ( FIND_IN_SET(tea3.value, @ENV_KNOWNFQDNS) OR tea3.value = @ENV_HOSTNAME ) )
        ORDER BY tel.id_asset_element LIMIT 1);
      RETURN myid;
END; //
DELIMITER ;

/* Note: due to MySQL security, we can not overwrite existing files,
 * nor use variables in SELECT INTO and SOURCE commands */
SET @str = IF (NOT EXISTS(SELECT 1 FROM mysql.proc p WHERE db = 'box_utf8' AND name = 'select_RC_myself'),
'CREATE FUNCTION select_RC_myself()  RETURNS INT UNSIGNED
  BEGIN
    RETURN select_RC_myself_default();
  END;', 'SET @dummy = 0;');
\! /bin/rm -f /tmp/0011_default_rc-func-myself.sql
SELECT @str INTO OUTFILE '/tmp/0011_default_rc-func-myself.sql';

DELIMITER //
SOURCE /tmp/0011_default_rc-func-myself.sql //
DELIMITER ;

\! /bin/rm -f /tmp/0011_default_rc-func-myself.sql



/* Define the logic to create or update the rack controller */
DROP PROCEDURE IF EXISTS addRC0 ;
DELIMITER //
CREATE PROCEDURE addRC0()
BEGIN
  START TRANSACTION;

    SET @id_type_dc = (SELECT DISTINCT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "datacenter" LIMIT 1);
    SET @id_type_device = (SELECT DISTINCT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "device" LIMIT 1);
    SET @id_subtype_rc = (SELECT DISTINCT id_asset_device_type FROM t_bios_asset_device_type WHERE name = "rack controller" LIMIT 1);

    SET @rcparent = (SELECT DISTINCT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_dc ORDER BY id_asset_element LIMIT 1);
    SET @rcmyself = select_RC_myself();
    SET @rc0present = (SELECT count(id_asset_element) FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
    SET @rc0id = (SELECT DISTINCT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
    SET @rccount = (SELECT count(id_asset_element) FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc ) ;

    SET @rc0added = NULL;
    IF @rc0present = 0 THEN
      IF @rcmyself IS NULL THEN
        SET @rc0added = TRUE;
        INSERT INTO t_bios_asset_element (name, id_type, id_subtype, id_parent, status, priority, asset_tag) VALUES ('rackcontroller-0', @id_type_device, @id_subtype_rc, @rcparent, 'active', 1, NULL);
        SET @rc0id = (SELECT DISTINCT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
        SET @rc0idLast = (SELECT LAST_INSERT_ID()) ;
        SET @rc0name = (SELECT IF(@ENV_HOSTNAME IS NOT NULL,@ENV_HOSTNAME,IF(@HARDWARE_CATALOG_NUMBER IS NOT NULL,@HARDWARE_CATALOG_NUMBER,'IPC 3000')));
        INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('name', @rc0name, @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('location_u_pos', '1', @rc0id, 0), ('u_size', '1', @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        SET @ipnum = 0;
        IF @ENV_IPADDRS IS NOT NULL THEN
          SET @arr = CONCAT(@ENV_IPADDRS, ',');
          SET @pos = LOCATE(',', @arr);
          WHILE (@pos > 0) DO
            SET @ipnum = @ipnum + 1;
            SET @val = LEFT(@arr, @pos - 1);
            SET @arr = SUBSTRING(@arr, @pos + 1);
            SET @pos = LOCATE(',', @arr);
            INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES (CONCAT('ip.', @ipnum), @val, @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
          END WHILE;
        END IF;
        IF @ipnum = 0 THEN
          INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('ip.1', '127.0.0.1', @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        END IF;
        IF @ENV_KNOWNFQDNS IS NOT NULL THEN
          SET @arr = CONCAT(@ENV_KNOWNFQDNS, ',');
          SET @pos = LOCATE(',', @arr);
          SET @val = LEFT(@arr, @pos - 1);
          INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('fqdn', @val, @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        END IF;
        IF @ENV_HARDWARE_SERIAL_NUMBER IS NOT NULL THEN
          INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('serial_no', @ENV_HARDWARE_SERIAL_NUMBER, @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        END IF;
        IF @ENV_HARDWARE_CATALOG_NUMBER IS NOT NULL THEN
          INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('model', CONCAT(@ENV_HARDWARE_CATALOG_NUMBER,IF(@HARDWARE_SPEC_REVISION IS NULL,'',CONCAT(' HW rev ',@HARDWARE_SPEC_REVISION))), @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        END IF;
        INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('manufacturer', 'Eaton', @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        IF @ENV_HARDWARE_UUID IS NOT NULL THEN
          INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('uuid', @ENV_HARDWARE_UUID, @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        END IF;
        INSERT INTO t_bios_asset_link (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) VALUES (IF(@rcparent IS NOT NULL,@rcparent,@rc0id), @rc0id, (SELECT DISTINCT id_asset_link_type FROM t_bios_asset_link_type WHERE name = "power chain" LIMIT 1), NULL, NULL);
        INSERT INTO t_bios_discovered_device (name, id_device_type) VALUES (@rc0name, (SELECT DISTINCT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1)) ON DUPLICATE KEY UPDATE id_discovered_device = LAST_INSERT_ID(id_discovered_device);
        INSERT INTO t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES ((SELECT DISTINCT id_discovered_device FROM t_bios_discovered_device WHERE name = @rc0name AND id_device_type = (SELECT DISTINCT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1) LIMIT 1), @rc0id);
      ELSE
        SET @rc0added = FALSE;
        UPDATE t_bios_asset_element SET name = 'rackcontroller-0' WHERE id_asset_element = @rcmyself;
        SET @rc0id = (SELECT DISTINCT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
      END IF;
    END IF;

  COMMIT;
END; //
DELIMITER ;

/* Add, update or skip (if already OK) a 'rackcontroller-0' named asset */
CALL addRC0;

/* Report the results in log */
SELECT @rc0present, @rc0added, @rc0id, @rcmyself, @rccount, @rcparent;
SELECT * FROM t_bios_asset_element WHERE id_asset_element IN (@rc0id, @rcparent);



/* Clean up */
DROP PROCEDURE IF EXISTS addRC0 ;
DROP FUNCTION IF EXISTS select_RC_myself;
DROP FUNCTION IF EXISTS select_RC_myself_default ;


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
