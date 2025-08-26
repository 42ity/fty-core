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

# Add ksmbd module to the blacklist
echo "\n# KSMBD" >> "$BLACKLIST_FILE"
echo "blacklist ksmbd" >> "$BLACKLIST_FILE"

# Add ath12k module to the blacklist
echo "\n# ATH12K" >> "$BLACKLIST_FILE"
echo "blacklist ath12k" >> "$BLACKLIST_FILE"

# Add generic wifi module to the blacklist
echo "\n# WIFI" >> "$BLACKLIST_FILE"
echo "blacklist iwlwifi" >> "$BLACKLIST_FILE"

# Add generic & each snd module to the blacklist
echo "\n# SND" >> "$BLACKLIST_FILE"
# List of snd-related modules to blacklist
SND_MODULES=$(lsmod | awk '/^snd/ {print $1}' | sort -u)
echo "blacklist snd" >> "$BLACKLIST_FILE"
for module in $SND_MODULES; do
    echo "blacklist $module" >> "$BLACKLIST_FILE"
done

# Add spi modules to the blacklist
echo "\n# SPI" >> "$BLACKLIST_FILE"
echo "blacklist spi_amd_platform" >> "$BLACKLIST_FILE"
echo "blacklist spi_bitbang" >> "$BLACKLIST_FILE"
echo "blacklist spi_bcm2835" >> "$BLACKLIST_FILE"
echo "blacklist spidev" >> "$BLACKLIST_FILE"


echo "Blacklisted modules written to $BLACKLIST_FILE"

exit 0
