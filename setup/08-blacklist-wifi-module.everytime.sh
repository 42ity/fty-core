#!/bin/sh

# Define the output file for blacklisting
BLACKLIST_FILE="/etc/modprobe.d/blacklist-wifi.conf"

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

# Create or overwrite the blacklist file
echo "# Blacklist wifi module" > "$BLACKLIST_FILE"

# Add generic wifi module to the blacklist
echo "blacklist iwlwifi" >> "$BLACKLIST_FILE"

echo "Blacklisted wifi module written to $BLACKLIST_FILE"
