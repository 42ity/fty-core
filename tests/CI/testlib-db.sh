#!/bin/sh
#
# Copyright (C) 2014-2015 Eaton
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
#! \file    testlib-db.sh
#  \brief   library of functions and strings useful for database manipulation
#           specifically in $BIOS testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \author  Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \details This is library of functions useful for $BIOS testing related to
#           databases, which can be sourced to interactive shell.
#           Generally it should not be included directly into a script because
#           it is sourced by weblib.sh along with testlib.sh; if you do need it
#           explicitly - include after scriptlib.sh, and after weblib.sh if you
#           want to use init_script*() which call accept_license().
#           Note: at least for now, many definitions relevant to database work
#           exist in other script-libraries because they first appeared there.
#           We may later choose to move them here, but it is not a priority.

# ***********************************************
### Database global variables
DB_LOADDIR="$CHECKOUTDIR/database/mysql"
DB_BASE="$DB_LOADDIR/initdb.sql"
DB_DATA="$DB_LOADDIR/load_data.sql"
DB_DATA_CURRENT="$DB_LOADDIR/current_data.sql"
DB_DATA_TESTREST="$DB_LOADDIR/load_data_test_restapi.sql"
DB_TOPOP="$DB_LOADDIR/power_topology.sql"
DB_TOPOL="$DB_LOADDIR/location_topology.sql"
DB_RACK_POWER="$DB_LOADDIR/rack_power.sql"
DB_DC_POWER="$DB_LOADDIR/dc_power.sql"
DB_DC_POWER_UC1="$DB_LOADDIR/ci-DC-power-UC1.sql"
DB_CRUD="$DB_LOADDIR/crud_test.sql"
DB_OUTAGE="$DB_LOADDIR/test_outage.sql"
DB_ALERT="$DB_LOADDIR/test_alert.sql"
DB_ASSET_TAG_NOT_UNIQUE="$DB_LOADDIR/initdb_ci_patch.sql"

### Some pre-sets for CSV tests
CSV_LOADDIR="$CHECKOUTDIR/tests/fixtures/csv"
CSV_LOADDIR_BAM="$CSV_LOADDIR/bam"
CSV_LOADDIR_TPOWER="$CSV_LOADDIR/tpower"
CSV_LOADDIR_ASSIMP="$CSV_LOADDIR/asset_import"

### Directories where we can dump some output (mysqldump, temporary data, etc.)
DB_DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"     # TODO: change to BUILDSUBDIR
DB_TMPSQL_DIR="/tmp"
#DB_TMPSQL_DIR="$DB_DUMP_DIR"

### Expected results (saved in Git) are stored here:
DB_RES_DIR="$CHECKOUTDIR/tests/CI/web/results"

### Killing connections as we recreate the database can help ensure that the
### old data would not survive and be referred to by subsequent tests which
### expect to start from a clean slate. But in practice some clients do die.
### Until we debug this to make them survive the database reconnections, the
### toggle defaults to "no". Even later it makes sense to keep this variable
### so we can have regression testing (that the ultimate fix works forever).
[ -z "${DB_KILL_CONNECTIONS-}" ] && DB_KILL_CONNECTIONS=no

do_killdb() {
    KILLDB_RES=0
    if [ -n "${DATABASE-}" ] ; then
        if [ x"$DB_KILL_CONNECTIONS" = xyes ]; then
            logmsg_warn "Trying to kill all connections to the ${DATABASE} database; some clients can become upset - it is their bug then!"
            sut_run 'mysql --disable-column-names -s -e "SHOW PROCESSLIST" | grep -vi PROCESSLIST | awk '"'\$4 ~ /$DATABASE/ {print \$1}'"' | while read P ; do mysqladmin kill "$P" || do_select "KILL $P" ; done' || KILLDB_RES=$?
        fi
        DATABASE=mysql do_select "DROP DATABASE ${DATABASE}" || \
        sut_run "mysqladmin drop -f ${DATABASE}" || \
        { KILLDB_RES=$? ; logmsg_error "Failed to DROP DATABASE" ; }
        sleep 1
    else
        logmsg_warn "The DATABASE variable is not set, nothing known to DROP"
    fi
    DATABASE=mysql do_select "RESET QUERY CACHE" || \
        logmsg_warn "Failed to RESET QUERY CACHE"
    DATABASE=mysql do_select "FLUSH QUERY CACHE" || \
        logmsg_warn "Failed to FLUSH QUERY CACHE"
    sut_run "mysqladmin refresh ; sync; [ -w /proc/sys/vm/drop_caches ] && echo 3 > /proc/sys/vm/drop_caches && sync" || \
        logmsg_warn "Failed to FLUSH OS/VM CACHE"
    return $KILLDB_RES
}

killdb() {
    echo "--------------- reset db: kill old DB ------------"
    KILLDB_OUT="`do_killdb 2>&1`"
    KILLDB_RES=$?
    if [ $KILLDB_RES != 0 ]; then
        logmsg_error "Hit some error while killing old database:"
        echo "==========================================="
        echo "$KILLDB_OUT"
        echo "==========================================="
    fi
    logmsg_info "Database should have been dropped and caches should have been flushed at this point"
    return $KILLDB_RES
}

loaddb_initial() {
    killdb
    echo "--------------- reset db: initialize -------------"
    for data in "$DB_BASE" ; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema should have been initialized at this point: core schema file only"
    return 0
}

loaddb_sampledata() {
    echo "--------------- reset db: default sample data ----"
    loaddb_initial || return $?
    for data in "$DB_DATA" ; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema and data should have been initialized at this point: sample datacenter for tests"
    return 0
}

loaddb_default() {
    echo "--------------- reset db: default REST API -------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_DATA_TESTREST"; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema and data should have been initialized at this point: for common REST API tests"
    return 0
}

loaddb_topo_loc() {
    echo "--------------- reset db: topo-location ----------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_TOPOL"; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema and data should have been initialized at this point: for topology-location tests"
    return 0
}

loaddb_topo_pow() {
    echo "--------------- reset db: topo-power -------------"
    loaddb_initial || return $?
    for data in "$DB_DATA" "$DB_TOPOP"; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema and data should have been initialized at this point: for topology-power tests"
    return 0
}

loaddb_current() {
    echo "--------------- reset db: current ----------------"
    loaddb_initial || return $?
    for data in "$DB_DATA_CURRENT"; do
    #for data in "$DB_DATA_CURRENT" "$DB_DATA_TESTREST"; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
    done
    logmsg_info "Database schema and data should have been initialized at this point: for current tests"
    return 0
}

init_script_initial(){
# Prepare sandbox for the test: ensure the database is freshly made
# and licenses to not interfere; the accept_license() routine is
# defined in weblib.sh at the moment
    loaddb_initial && \
    accept_license
}

init_script_sampledata(){
    loaddb_sampledata && \
    accept_license
}

init_script_default(){
    loaddb_default && \
    accept_license
}

init_script(){
    init_script_default "$@"
}

init_script_topo_loc(){
    loaddb_topo_loc && \
    accept_license
}

init_script_topo_pow(){
    loaddb_topo_pow && \
    accept_license
}

