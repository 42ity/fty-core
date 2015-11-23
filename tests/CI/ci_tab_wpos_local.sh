#/!bin/sh
#
# Copyright (C) 2014 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file   ci_tab_wpos_local.sh
#  \brief  Not yet documented file
#  \author Radomir Vrajik <RadomirVrajik@Eaton.com>

SUT_HOST="127.0.0.1"
SUT_WEB_PORT=8000
SUT_IS_REMOTE=no
BASE_URL="http://$SUT_HOST:$SUT_WEB_PORT/api/v1"


# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
# Include our standard web routines for CI scripts
. "`dirname $0`"/weblib.sh || \
    { echo "CI-FATAL: $0: Can not web script library" >&2; exit 1; }

#SUT_SSH_PORT="2209"
#SUT_HOST="debian.roz53.lab.etn.com"
#SUT_WEB_PORT=8009
#SUT_IS_REMOTE=no

cd `dirname $0`;cd ../..;CHECKOUTDIR=`pwd`
echo "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
set -u

# Import empty DB
DB1="/home/rvrajik/core/tools/initdb.sql"
#mysql -u root < "$DB1"
#mysql -u root < /home/rvrajik/core/tools/initdb.sql
#mysql -u root box_utf8 <<< "select * from t_bios_asset_element_type"
#mysql -u root box_utf8 <<< "delete from t_bios_asset_device_type"
#mysql -u root box_utf8 <<< "delete from t_bios_asset_link_type"

loaddb_file $DB1

ASSET="$CHECKOUTDIR/tools/$1"
# Import the tools/<testname>.csv file
# <testname> format bam_import_16_vte_wpos<N>.csv, where <N> is tcnumber
# bam_import_16_wpos1.csv - 2 epdu's + 1 pdu in the same rack
# expected - [ 8, "more than 2 PDU is not supported"]
# 
# bam_import_16_wpos2.csv - parameter is missing
# expected
#            [ 4, "location_w_pos should be set. Possible variants 'left'/'right'"],
#            [ 5, "location_w_pos should be set. Possible variants 'left'/'right'"],
#            [ 12, "location_w_pos should be set. Possible variants 'left'/'right'"],
#            [ 13, "location_w_pos should be set. Possible variants 'left'/'right'"] 
#
# bam_import_16_wpos3.csv - all items OK
# expected - no errors
#
# bam_import_16_wpos4.csv - both epdu's are left
# expected - "left" of the second is changed to "right"
# and warning in log - "location_w_pow changed to 'right'"


api_auth_post_file /asset/import assets=@$ASSET -H "Expect:"
mysql -u root box_utf8 <<< "select * from t_bios_asset_element_type"


