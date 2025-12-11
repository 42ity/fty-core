#!/bin/sh

#
#   Copyright (c) 2019 - 2020 Eaton
#
#   This file is part of the Eaton 42ity project.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program; if not, write to the Free Software Foundation, Inc.,
#   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

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

# Add nfs module to the blacklist (nfs-kernel-server)
echo "\n# NFS" >> "$BLACKLIST_FILE"
echo "blacklist nfsd" >> "$BLACKLIST_FILE"

echo "Blacklisted modules written to $BLACKLIST_FILE"

exit 0
