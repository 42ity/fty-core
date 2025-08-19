#!/bin/sh

# Define the output file for blacklisting
BLACKLIST_FILE="/etc/modprobe.d/blacklist-snd.conf"

# List of snd-related modules to blacklist
SND_MODULES=$(lsmod | awk '/^snd/ {print $1}' | sort -u)

# Check if running as root
if [ "$(id -u)" -ne 0 ]; then
    echo "This script must be run as root."
    exit 1
fi

# Create or overwrite the blacklist file
echo "# Blacklist all snd modules" > "$BLACKLIST_FILE"

# Add generic & each snd module to the blacklist
echo "blacklist snd" >> "$BLACKLIST_FILE"
for module in $SND_MODULES; do
    echo "blacklist $module" >> "$BLACKLIST_FILE"
done

echo "Blacklisted snd modules written to $BLACKLIST_FILE"
