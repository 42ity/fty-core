
#
# Copyright (C) 2015 Eaton
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
#! \file alerts.sh
#  \author Michal Vyskocil <MichalVyskocil@Eaton.com>
#  \brief Not yet documented file

echo
echo "###################################################################################################"
echo "********* alerts.sh ******************************** START ****************************************"
echo "###################################################################################################"
echo

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
loaddb_default

echo "***************************************************************************************************"
echo "********* 1. list_of_alerts ***********************************************************************"
echo "***************************************************************************************************"
test_it "list_of_alerts"
api_get_json /alerts >&5
print_result $?

echo
echo "###################################################################################################"
echo "********* alerts.sh ******************************** END ******************************************"
echo "###################################################################################################"
echo
