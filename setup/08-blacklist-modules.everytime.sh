#!/bin/sh

# Define the output file for blacklisting
BLACKLIST_FILE="/etc/modprobe.d/blacklist-ipm2.conf"

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

# Create or overwrite the blacklist file
echo "# Blacklist modules" > "$BLACKLIST_FILE"


// ATH12K

# Add ath12k module to the blacklist
echo "blacklist ath12k" >> "$BLACKLIST_FILE"


// WIFI

# Add generic wifi module to the blacklist
echo "blacklist iwlwifi" >> "$BLACKLIST_FILE"


// SND

# List of snd-related modules to blacklist
SND_MODULES=$(lsmod | awk '/^snd/ {print $1}' | sort -u)
# Add generic & each snd module to the blacklist
echo "blacklist snd" >> "$BLACKLIST_FILE"
for module in $SND_MODULES; do
    echo "blacklist $module" >> "$BLACKLIST_FILE"
done

//


echo "Blacklisted modules written to $BLACKLIST_FILE"
