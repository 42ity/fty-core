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
#           explicitly - include after scriptlib.sh, and after weblib.sh if you
#	    want to use init_script*() which call accept_license().

# ***********************************************
### Database global variables
DB_LOADDIR="$CHECKOUTDIR/tools"
DB_BASE="$DB_LOADDIR/initdb.sql"
DB_DATA="$DB_LOADDIR/load_data.sql"
DB_DATA_CURRENT="$DB_LOADDIR/current_data.sql"
DB_DATA_TESTREST="$DB_LOADDIR/load_data_test_restapi.sql"
DB_TOPOP="$DB_LOADDIR/power_topology.sql"
DB_TOPOL="$DB_LOADDIR/location_topology.sql"
DB_ASSET_TAG_NOT_UNIQUE="$DB_LOADDIR/initdb_ci_patch.sql"

### Directories where we can dump some output (mysqldump, temporary data, etc.)
DB_DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"     # TODO: change to BUILDSUBDIR
DB_TMPSQL_DIR="/tmp"
#DB_TMPSQL_DIR="$DB_DUMP_DIR"

### Expected results (saved in Git) are stored here:
DB_RES_DIR="$CHECKOUTDIR/tests/CI/web/results"

killdb() {
    echo "--------------- reset db: kill old DB ------------"
    if [ -n "${DATABASE-}" ] ; then
        sut_run 'mysql --disable-column-names -s -e "SHOW PROCESSLIST" | grep -vi PROCESSLIST | awk '"'\$4 ~ /$DATABASE/ {print \$1}'"' | while read P ; do mysqladmin kill "$P" || do_select "KILL $P" ; done'
        DATABASE=mysql do_select "DROP DATABASE ${DATABASE}" || true
        sut_run "mysqladmin drop ${DATABASE}"
    fi
    DATABASE=mysql do_select "RESET QUERY CACHE" || true
    DATABASE=mysql do_select "FLUSH QUERY CACHE" || true
    sut_run "mysqladmin refresh"
    sut_run "sync; [ -w /proc/sys/vm/drop_caches ] && echo 3 > /proc/sys/vm/drop_caches && sync"
    return 0
}

loaddb_initial() {
    killdb
    echo "--------------- reset db: initialize -------------"
    for data in "$DB_BASE" ; do
        loaddb_file "$data" || return $?
    done
    return 0
}

loaddb_default() {
    echo "--------------- reset db: default ----------------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_DATA_TESTREST"; do
        loaddb_file "$data" || return $?
    done
    return 0
}

loaddb_topo_loc() {
    echo "--------------- reset db: topo-location ----------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_TOPOL"; do
        loaddb_file "$data" || return $?
    done
    return 0
}

loaddb_topo_pow() {
    echo "--------------- reset db: topo-power -------------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_TOPOP"; do
        loaddb_file "$data" || return $?
    done
    return 0
}

loaddb_current() {
    echo "--------------- reset db: current ----------------"
    loaddb_initial || return $?
    for data in "$DB_DATA_CURRENT"; do
    #for data in "$DB_DATA_CURRENT" "$DB_DATA_TESTREST"; do
        loaddb_file "$data" || return $?
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

