#!/bin/bash

echo "Check requirements of environment for BIOS when cloning & building:"

# Check no. 1:
# We expect db to be in UTC time zone
OUT=$(mysql -u root -N -e "select FROM_UNIXTIME (1577890800)")
if ! grep -q "2020-01-01 15:00:00" <<< "${OUT}"; then
    echo "ISSUE: Fix database time zone!"
    exit 1
fi

OUT=$(mysql -u root -N -e "select FROM_UNIXTIME (1437137193)")
if ! grep -q "2015-07-17 12:46:33" <<< "${OUT}"; then
    echo "ISSUE: Fix database time zone!"
    exit 1
fi

echo "OK."
