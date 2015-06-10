#!/bin/bash

# TODO: Check nonempty input, write usage when input bad

ELEMENT_ID=$1
SOURCE=$2

ID=$(mysql -u root -D box_utf8 -N -e "    
SELECT @id := id FROM t_bios_measurement_topic,
    (
    SELECT @device_id := a.id_discovered_device, @name := name
    FROM
        t_bios_discovered_device AS a LEFT JOIN t_bios_monitor_asset_relation AS b
        ON a.id_discovered_device = b.id_discovered_device
    WHERE
        id_asset_element = '${1}'
    ) AS c
WHERE
    device_id = @device_id AND
    topic = CONCAT('${2}@', @name)")

echo "$ID"    

