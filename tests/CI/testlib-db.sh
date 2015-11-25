#!/bin/sh
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
#! \file    testlib.sh
#  \brief   library of functions and strings useful for database manipulation
#           specifically in $BIOS testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \details This is library of functions useful for $BIOS testing related to
#           databases, which can be sourced to interactive shell.
#           Generally it should not be included directly into a script because
#           it is sourced by weblib.sh along with testlib.sh; if you do need it
#           explicitly - include after scriptlib.sh and weblib.sh.

# ***********************************************
### Database global variables
DB_LOADDIR="$CHECKOUTDIR/tools"
DB_BASE="initdb.sql"
DB_DATA="load_data.sql"
DB_DATA_TESTREST="load_data_test_restapi.sql"
DB_TOPOP="power_topology.sql"
DB_TOPOL="location_topology.sql"

### Directories where we can dump some output (mysqldump, temporary data, etc.)
DB_DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"     # TODO: change to BUILDSUBDIR
DB_TMPSQL_DIR="/tmp"
#DB_TMPSQL_DIR="$DB_DUMP_DIR"

### Expected results (saved in Git) are stored here:
DB_RES_DIR="$CHECKOUTDIR/tests/CI/web/results"

loaddb_initial() {
    echo "--------------- reset db: initial ----------------"
    for data in "$DB_BASE" ; do
        loaddb_file "$DB_LOADDIR/$data" || return $?
    done
    return 0
}

loaddb_default() {
    echo "--------------- reset db: default ----------------"
    for data in "$DB_BASE" "$DB_DATA" "$DB_DATA_TESTREST"; do
        loaddb_file "$DB_LOADDIR/$data" || return $?
    done
    return 0
}

loaddb_topo_loc() {
    echo "--------------- reset db: topo-location ----------"
    for data in "$DB_BASE" "$DB_DATA" "$DB_TOPOL"; do
        loaddb_file "$DB_LOADDIR/$data" || return $?
    done
    return 0
}

loaddb_topo_pow() {
    echo "--------------- reset db: topo-power -------------"
    for data in "$DB_BASE" "$DB_DATA" "$DB_TOPOP"; do
        loaddb_file "$DB_LOADDIR/$data" || return $?
    done
    return 0
}

init_script(){
# Prepare sandbox for the test: ensure the database is freshly made
# and licenses to not interfere; the accept_license() routine is
# defined in weblib.sh at the moment
    loaddb_default
    accept_license
}

init_script_topo_loc(){
    loaddb_topo_loc
    accept_license
}

init_script_topo_pow(){
    loaddb_topo_pow
    accept_license
}

