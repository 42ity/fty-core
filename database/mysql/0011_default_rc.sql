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
\! /bin/rm -f /tmp/fty-envvars.sql
\! /usr/share/bios/scripts/envvars-ExecStartPre.sh -O /tmp/fty-envvars.sql --sql
SOURCE /tmp/fty-envvars.sql ;
\! /bin/rm -f /tmp/fty-envvars.sql

/* Define an overridable manner of selecting a "myself" rack controller
 * to pick one from several available, which we can evolve for future
 * versions (e.g. based on known UUID, IP, etc.) - just make sure this
 * FUNCTION select_RC_myself() gets defined earlier. This only gets
 * used if we do at all need to upgrade existing database contents. */
/* Note: due to MySQL security, we can not overwrite existing files,
 * nor use variables in SELECT INTO and SOURCE commands */
SET @str = IF (NOT EXISTS(SELECT 1 FROM mysql.proc p WHERE db = 'box_utf8' AND name = 'select_RC_myself'),
'CREATE FUNCTION select_RC_myself()  RETURNS INT UNSIGNED
  BEGIN
      DECLARE myid INT UNSIGNED;
      SET myid = (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc ORDER BY id_asset_element LIMIT 1);
      RETURN myid;
  END;', 'SET @dummy = 0;');

\! /bin/rm -f /tmp/zzz.sql
SELECT @str INTO OUTFILE '/tmp/zzz.sql';

DELIMITER //
SOURCE /tmp/zzz.sql //
DELIMITER ;

\! /bin/rm -f /tmp/zzz.sql



/* Define the logic to create or update the rack controller */
DROP PROCEDURE IF EXISTS addRC0 ;
DELIMITER //
CREATE PROCEDURE addRC0()
BEGIN
  START TRANSACTION;

    SET @id_type_dc = (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "datacenter" LIMIT 1);
    SET @id_type_device = (SELECT id_asset_element_type FROM t_bios_asset_element_type WHERE name = "device" LIMIT 1);
    SET @id_subtype_rc = (SELECT id_asset_device_type FROM t_bios_asset_device_type WHERE name = "rack controller" LIMIT 1);

    SET @rcparent = (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_dc ORDER BY id_asset_element LIMIT 1);
    SET @rcmyself = select_RC_myself();
    SET @rc0present = (SELECT count(id_asset_element) FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
    SET @rc0id = (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
    SET @rccount = (SELECT count(id_asset_element) FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc ) ;

    SET @rc0added = NULL;
    IF @rc0present = 0 THEN
      IF @rccount = 0 THEN
        SET @rc0added = TRUE;
        INSERT INTO t_bios_asset_element (name, id_type, id_subtype, id_parent, status, priority, asset_tag) VALUES ('rackcontroller-0', @id_type_device, @id_subtype_rc, @rcparent, 'active', 1, NULL);
        SET @rc0id = (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
        SET @rc0idLast = (SELECT LAST_INSERT_ID()) ;
        INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('name', 'IPC 3000', @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        INSERT INTO t_bios_asset_ext_attributes (keytag, value, id_asset_element, read_only) VALUES ('location_u_pos', '1', @rc0id, 0), ('u_size', '1', @rc0id, 0), ('ip.1', '127.0.0.1', @rc0id, 0) ON DUPLICATE KEY UPDATE id_asset_ext_attribute = LAST_INSERT_ID(id_asset_ext_attribute);
        INSERT INTO t_bios_asset_link (id_asset_device_src, id_asset_device_dest, id_asset_link_type, src_out, dest_in) VALUES (IF(@rcparent IS NOT NULL,@rcparent,@rc0id), @rc0id, (SELECT id_asset_link_type FROM t_bios_asset_link_type WHERE name = "power chain" LIMIT 1), NULL, NULL);
        INSERT INTO t_bios_discovered_device (name, id_device_type) VALUES ('IPC 3000', (SELECT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1)) ON DUPLICATE KEY UPDATE id_discovered_device = LAST_INSERT_ID(id_discovered_device);
        INSERT INTO t_bios_monitor_asset_relation (id_discovered_device, id_asset_element) VALUES ((SELECT id_discovered_device FROM t_bios_discovered_device WHERE name = 'IPC 3000' AND id_device_type = (SELECT id_device_type FROM t_bios_device_type WHERE name = "not_classified" LIMIT 1) LIMIT 1), @rc0id);
      ELSE
        SET @rc0added = FALSE;
        UPDATE t_bios_asset_element SET name = 'rackcontroller-0' WHERE id_asset_element = @rcmyself;
        SET @rc0id = (SELECT id_asset_element FROM t_bios_asset_element WHERE id_type = @id_type_device AND id_subtype = @id_subtype_rc AND name = 'rackcontroller-0' ) ;
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


/* This must be the last line of the SQL file */
START TRANSACTION;
INSERT INTO t_bios_schema_version (tag,timestamp,filename,version) VALUES('finish-import', UTC_TIMESTAMP() + 0, @bios_db_schema_filename, @bios_db_schema_version);
/* Report the value */
SELECT * FROM t_bios_schema_version WHERE tag = 'finish-import' order by id desc limit 1;
COMMIT;
