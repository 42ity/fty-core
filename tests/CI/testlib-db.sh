#
# Copyright (C) 2014 - 2020 Eaton
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
#           specifically in 42ity testing
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author  Karol Hrdina <KarolHrdina@Eaton.com>
#  \author  Radomir Vrajik <RadomirVrajik@Eaton.com>
#  \details This is library of functions useful for 42ity testing related to
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
# DB_BASE_PATCHes are part of standard schema and so they should be
# loaded along with initdb.sql wherever the empty structure is needed
DB_BASE="$DB_LOADDIR/initdb.sql"
DB_BASE_PATCH0001="$DB_LOADDIR/0001_device_type_extension.sql"
DB_BASE_PATCH0002="$DB_LOADDIR/0002_v_web_element_view.sql"
DB_BASE_PATCH0003="$DB_LOADDIR/0003_device_type_extension.sql"
DB_BASE_PATCH0004="$DB_LOADDIR/0004_ext_cascade.sql"
DB_BASE_PATCH0005="$DB_LOADDIR/0005_super_parent_names.sql"
DB_BASE_PATCH0006="$DB_LOADDIR/0006_super_parent_names.sql"
DB_BASE_PATCH0007="$DB_LOADDIR/0007_device_type_extension.sql"
DB_BASE_PATCH0008="$DB_LOADDIR/0008_ext_names.sql"
DB_BASE_PATCH0009="$DB_LOADDIR/0009_device_type_extension.sql"
DB_BASE_PATCH0010="$DB_LOADDIR/0010_super_parent_names.sql"
DB_BASE_PATCH0011="$DB_LOADDIR/0011_default_rc.sql"
# Note: This approach requires whitespace-free DB_LOADDIR value
# TODO: Generate the list with `ls` or asterisk?
DB_BASE_PATCHES="$DB_BASE_PATCH0001 $DB_BASE_PATCH0002 $DB_BASE_PATCH0003 $DB_BASE_PATCH0004 $DB_BASE_PATCH0005 $DB_BASE_PATCH0006 $DB_BASE_PATCH0007 $DB_BASE_PATCH0008 $DB_BASE_PATCH0009 $DB_BASE_PATCH0010 $DB_BASE_PATCH0011"
export DB_LOADDIR DB_BASE DB_BASE_PATCHES

# Sample data sets for some tests
DB_DATA="$DB_LOADDIR/load_data.sql"
DB_DATA_CURRENT="$DB_LOADDIR/current_data.sql"
DB_DATA_TESTREST="$DB_LOADDIR/load_data_test_restapi.sql"
export DB_DATA

DB_TOPOP_NAME="power_topology.sql"
DB_TOPOP="$DB_LOADDIR/$DB_TOPOP_NAME"

DB_TOPOL_NAME="location_topology.sql"
DB_TOPOL="$DB_LOADDIR/$DB_TOPOL_NAME"

DB_RACK_POWER_NAME="rack_power.sql"
DB_RACK_POWER="$DB_LOADDIR/$DB_RACK_POWER_NAME"

DB_DC_POWER_NAME="dc_power.sql"
DB_DC_POWER="$DB_LOADDIR/$DB_DC_POWER_NAME"

DB_DC_POWER_UC1="$DB_LOADDIR/ci-DC-power-UC1.sql"
DB_CRUD="$DB_LOADDIR/crud_test.sql"
DB_OUTAGE="$DB_LOADDIR/test_outage.sql"
DB_ASSET_TAG_NOT_UNIQUE="$DB_LOADDIR/initdb_ci_patch.sql"

DB_AVERAGES="$DB_LOADDIR/test_averages.sql"
DB_AVERAGES_RELATIVE="$DB_LOADDIR/test_averages_relative.sql"

### Some pre-sets for CSV tests
CSV_LOADDIR="$CHECKOUTDIR/tests/fixtures/csv"
CSV_LOADDIR_BAM="$CSV_LOADDIR/bam"
CSV_LOADDIR_TPOWER="$CSV_LOADDIR/tpower"
CSV_LOADDIR_ASSIMP="$CSV_LOADDIR/asset_import"

### Directories where we can dump some output (mysqldump, temporary data, etc.)
DB_DUMP_DIR="$CHECKOUTDIR/tests/CI/web/log"     # TODO: change to BUILDSUBDIR
DB_TMPSQL_DIR="/tmp"
#DB_TMPSQL_DIR="$DB_DUMP_DIR"

### We also have to write exported temporary CSVs somewhere
CSV_TMP_DIR="$DB_DUMP_DIR"

### Expected results (saved in Git) are stored here:
DB_RES_DIR="$CHECKOUTDIR/tests/CI/web/results"

export DB_DUMP_DIR DB_TMPSQL_DIR DB_RES_DIR CSV_TMP_DIR
export CSV_LOADDIR CSV_LOADDIR_TPOWER CSV_LOADDIR_ASSIMP CSV_LOADDIR_BAM

### Killing connections as we recreate the database can help ensure that the
### old data would not survive and be referred to by subsequent tests which
### expect to start from a clean slate. But in practice some clients do die.
### Until we debug this to make them survive the database reconnections, the
### toggle defaults to "no". Even later it makes sense to keep this variable
### so we can have regression testing (that the ultimate fix works forever).
[ -z "${DB_KILL_CONNECTIONS-}" ] && DB_KILL_CONNECTIONS=no

### By default, do we use tarballs or SQL to reinitialize the database?
### See reloaddb_should_tarball() as the callable method for runtime check.
[ -z "${LOADDB_SHOULD_TARBALL-}" ] && LOADDB_SHOULD_TARBALL=auto
[ x"${LOADDB_SHOULD_TARBALL-}" = x- ] && LOADDB_SHOULD_TARBALL=auto

do_flushfs() {
    sut_run "sync; [ -w /proc/sys/vm/drop_caches ] && echo 3 > /proc/sys/vm/drop_caches && sync" || \
        logmsg_warn "Failed to FLUSH OS/VM CACHE"
}

do_flushdb() {
    # Ensure database reaches the disk storage and quiesces so we can do stuff
    # This also reduces chance of mysql service timeout to stop...
    DATABASE=mysql do_select "RESET QUERY CACHE" || \
        logmsg_warn "Failed to RESET QUERY CACHE"
    DATABASE=mysql do_select "FLUSH QUERY CACHE" || \
        logmsg_warn "Failed to FLUSH QUERY CACHE"
    sut_run "mysqladmin refresh" || \
        logmsg_warn "Failed to MYSQLADMIN REFRESH"
}

do_killdb() {
    KILLDB_RES=0
    if [ -n "${DATABASE-}" ] ; then
        if [ x"$DB_KILL_CONNECTIONS" = xyes ]; then
            logmsg_warn "Trying to kill all connections to the ${DATABASE} database; some clients can become upset - it is their bug then!"
            sut_run 'mysql --disable-column-names -s -e "SHOW PROCESSLIST" | grep -vi PROCESSLIST | awk '"'\$4 ~ /$DATABASE/ {print \$1}'"' | while read P ; do mysqladmin kill "$P" || do_select "KILL $P" ; done' || KILLDB_RES=$?
        fi
        DATABASE=mysql do_select "DROP DATABASE if exists ${DATABASE}" || \
        sut_run "mysqladmin drop -f ${DATABASE}" || \
        { KILLDB_RES=$? ; logmsg_error "Failed to DROP DATABASE" ; }
        sleep 1
    else
        logmsg_warn "The DATABASE variable is not set, nothing known to DROP"
    fi
    do_flushdb
    do_flushfs
    return $KILLDB_RES
}

killdb() {
    echo "CI-TESTLIB_DB - reset db: kill old DB ------------"
    KILLDB_OUT="`do_killdb 2>&1`"
    KILLDB_RES=$?
    if [ $KILLDB_RES != 0 ]; then
        logmsg_error "Hit some error while killing old database:"
        echo "==========================================="
        echo "$KILLDB_OUT"
        echo "==========================================="
    fi
    logmsg_debug "Database should have been dropped and caches should have been flushed at this point"
    return $KILLDB_RES
}

wipedb() (
    # This routine deletes all data except "dictionaries" via SQL
    # Intimate code snatched from continuous-integration/rest-py/biosrestapi/biosdatabase.py
    WIPEDB_RES=255
    if [ -n "${DATABASE-}" ] ; then
        WIPEDB_RES=0
        echo "CI-TESTLIB_DB - reset db: wipe database contents ---"
        for TABLE in \
                "t_bios_monitor_asset_relation" \
                "t_bios_asset_ext_attributes" \
                "t_bios_asset_link" \
                "t_bios_asset_group_relation" \
        ; do do_select "DELETE FROM $TABLE ; ALTER TABLE $TABLE AUTO_INCREMENT=0 ;" || WIPEDB_RES=$? ; done

        # Loop over nested assets until we've wiped them all
        ITER=1
        COUNT=-1
        PREVCOUNT=-2
        while [ "$COUNT" != "$PREVCOUNT" ] && [ "$COUNT" -ge -1 ] ; do
            PREVCOUNT="$COUNT"
            logmsg_debug "Deleting assets: round #$ITER"
            COUNT="`do_select 'select count(*) as cnt from t_bios_asset_element'`" || { WIPEDB_RES=$?; COUNT=-9; }
            [ "$COUNT" = 0 ] && break
            ITER="`expr $ITER + 1`"
            # Note: selecting and deleting must be done as separate SQL queries
            IDS=""
            IDLIST="`do_select 'SELECT id_asset_element FROM t_bios_asset_element WHERE id_asset_element NOT IN ( SELECT id_parent FROM t_bios_asset_element WHERE id_parent IS NOT NULL)'`" || { WIPEDB_RES=$?; IDLIST=""; COUNT=-9; }
            [ -n "$IDLIST" ] && for i in $IDLIST ; do
                [ -z "$IDS" ] && IDS="$i" || IDS="$IDS,$i"
            done
            [ -n "$IDS" ] && \
                do_select 'DELETE FROM t_bios_asset_element WHERE id_asset_element IN ( '"$IDS"' )' || WIPEDB_RES=$?
        done
        case "$COUNT" in
            0) logmsg_debug "Deleted all assets (0 left) after $ITER round(s)" ;;
            -*) logmsg_error "Something went wrong, could not delete all assets (code $COUNT) after $ITER round(s)" ;;
            *) logmsg_warn "Something went wrong, could not delete all assets ($COUNT were left) after $ITER round(s)" ;;
        esac

        for TABLE in \
                "t_bios_asset_element" \
                "t_bios_agent_info" \
                "t_bios_alert_device" \
                "t_bios_alert" \
                "t_bios_discovered_device" \
                "t_bios_measurement" \
                "t_bios_measurement_topic" \
        ; do do_select "DELETE FROM $TABLE ; ALTER TABLE $TABLE AUTO_INCREMENT=0 ;" || WIPEDB_RES=$? ; done

        [ "$WIPEDB_RES" != 0 ] && logmsg_error "Failed to WIPE DATABASE"

        do_flushdb
#       do_flushfs
    else
        logmsg_warn "The DATABASE variable is not set, nothing known to WIPE"
    fi
    return $WIPEDB_RES
)

tarballdb_export() {
    # Saves a tarball of the database files (/var/lib/mysql) under $DB_DUMP_DIR
    # as "$1.tgz" file (fast gzip-1). The DB server should be stopped and FS
    # synced by this time, to be provided externally (by caller).
    # NOTE: It might be faster to pass traffic from ARM to X86 uncompressed,
    # and pack it on the test-driver machine. Now the tested system compresses
    # and network traffic is minimized (but ARM CPU is slower at this).
    # TODO: Perhaps implement a smarter logic here that would consider both
    # connectivity and (assumed) performance of the tested and testing systems.
    sut_run "cd /var/lib && tar cf - -I'gzip -1' mysql/" > "$DB_DUMP_DIR/$1.tgz"
}

tarballdb_import() {
    # Uses a tarball of the database files $DB_DUMP_DIR/$1.tgz to repopulate
    # /var/lib/mysql. The DB server should be stopped and FS synced by this
    # time, to be provided externally (by caller). Caller restarts server too.
    # NOTE: We do 'sync' the writes here, so the database server startup does
    # not time out because I/O is slow at that time. Testing showed that delays
    # are the same whether we sync explicitly or absorb writes during restart.
    sut_run "cd /var/lib && rm -rf mysql && tar xzf - && sync" < "$DB_DUMP_DIR/$1.tgz"
}

tarballdb_newer() (
    # Compares $DB_DUMP_DIR/$1.tgz timestamp to timestamps of SQL files listed
    # in $2..$N (relative to $DB_LOADDIR or using explicit full paths).
    # Returns 0 if the tarball exists and is newer than SQLs it was built from.
    # Returns 1 if tarball is absent; 2 if any SQL is found to be newer than
    # the tarball or the SQL files are not listed or at least one is not found
    # in practice.
    # Non-zero result means that tarball should be (re)created.
    # Only errors like absent args or files are reported, not discrepancies.
    # NOTE that both the tarball and SQL files are assumed to be on this host
    # (the test-driving system).
    _PWD="`pwd`"
    SQLS=""
    TGZ="$DB_DUMP_DIR/$1.tgz"
    [ -s "$TGZ" ] || return 1
    shift
    [ $# -eq 0 ] && logmsg_error "tarballdb_newer(): got an empty argument" && return 2
    while [ $# -gt 0 ]; do
        case "$1" in
            "") logmsg_error "tarballdb_newer(): got an empty argument"; return 2 ;;
            /*) SQLS="$SQLS $1" ;;
            ./*|../*) SQLS="$SQLS $_PWD/$1" ;;
            *)  # Short filename - assume relative to loaddir
                [ -n "$DB_LOADDIR" ] && [ -d "$DB_LOADDIR" ] || logmsg_warn "DB_LOADDIR='$DB_LOADDIR' not found"
                SQLS="$SQLS $DB_LOADDIR/$1"
                ;;
        esac
        shift
    done
    [ -z "$SQLS" ] && logmsg_error "tarballdb_newer(): got no SQLs" && return 2
    for F in $SQLS ; do
        [ -s "$F" ] || { logmsg_error "tarballdb_newer(): SQL file '$F' not found"; return 2; }
        OUT="`find "$F" -type f -newer "$TGZ"`" || return 2
        [ -n "$OUT" ] && return 2
    done
    # TGZ exists, all (1+) SQLs exist/findable, and none is newer than the TGZ
    return 0
)

# TODO: Properly wrap as a method routine
_SUT_ARCH=""
reloaddb_should_tarball() {
    # Unless explicitly set by user, guess if we should use tarballs (SUT is ARM)
    # or it would be quicker to skip them (X86, nonzero return)
    if [ "$LOADDB_SHOULD_TARBALL" = auto ] || [ x"$LOADDB_SHOULD_TARBALL" = x- ]; then
        _SUT_ARCH="`sut_run 'uname -m'`" || _SUT_ARCH=""
        case "$_SUT_ARCH" in
            *arm*) LOADDB_SHOULD_TARBALL=yes ;;
            *x86*) LOADDB_SHOULD_TARBALL=no ;;
            *)     LOADDB_SHOULD_TARBALL=auto ;;
        esac
    fi
    [ "$LOADDB_SHOULD_TARBALL" = yes ] && return 0
    [ "$LOADDB_SHOULD_TARBALL" = no ] && return 1
    LOADDB_SHOULD_TARBALL=auto
    # To be on the safe side, while undecided, we cause proper SQL imports
    return 2
}

tarballdb_fastload() (
    # If the database $2 was tarballed and newer than constituent SQLs $3..$N,
    # stop SQL, import the tarball, restartSQL.
    # Progress reported with short text in "$1"
    # If not existant, not newer, or import failed - return non-zero so usual
    # SQL import can take place in the caller.
    reloaddb_should_tarball || return $?

    _DB_TXT="$1" ; shift
    _DB_TAG="$1-${_SUT_ARCH}" ; shift
    tarballdb_newer "${_DB_TAG}" "$@" || return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} via tarball '$DB_DUMP_DIR/${_DB_TAG}.tgz' --------"
    do_flushdb || true
    sut_run "/bin/systemctl stop mysql || /bin/systemctl stop mysql" && \
    tarballdb_import "${_DB_TAG}" && \
    sut_run "/bin/systemctl restart mysql"
    RES=$?
    [ $RES != 0 ] && logmsg_error "FAILED to un-tarball and/or restart the database!"
    return $RES
)

tarballdb_fastsave() (
    # Wraps the stop SQL, export database tagged "$1", restart SQL
    reloaddb_should_tarball || return 0

    _DB_TAG="$1-${_SUT_ARCH}" ; shift

    echo "CI-TESTLIB_DB - reset db: tarballing '${_DB_TAG}' into '$DB_DUMP_DIR/${_DB_TAG}.tgz' for future reference"
    do_flushdb || true
    do_flushfs || true
    mkdir -p "$DB_DUMP_DIR"
    sut_run "/bin/systemctl stop mysql || /bin/systemctl stop mysql" && \
    tarballdb_export "${_DB_TAG}" && \
    sut_run "/bin/systemctl restart mysql"
    RES=$?
    [ $RES != 0 ] && logmsg_error "FAILED to tarball and/or restart the database!"
    return $RES
)

do_loaddb_list() {
    if [ $# = 0 ] ; then
        logmsg_error "do_loaddb_list() called without arguments"
        return 1
    fi
    for data in "$@" ; do
        logmsg_info "Importing $data ..."
        loaddb_file "$data" || return $?
        logmsg_info "file $data applied OK"
    done
    return 0
}

loaddb_list() {
    LOADDB_OUT="`do_loaddb_list "$@" 2>&1`"
    LOADDB_RES=$?
    if [ $LOADDB_RES != 0 ]; then
        logmsg_error "Hit some error while importing database file(s):"
        echo "==========================================="
        echo "$LOADDB_OUT"
        echo "==========================================="
    else
        for data in "$@" ; do
            logmsg_info "file $data applied OK"
        done
    fi
    return $LOADDB_RES
}

loaddb_initial() (
    # Sub-processed to avoid namespace clashes with other loaddb_something routines
    _DB_TXT="(re-)initializing"
    _DB_TAG="loaddb_initial"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES && return $?

    sut_run "/bin/systemctl start mysql"
    killdb || true      # Would fail the next step, probably
    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} --------"
    loaddb_list "$DB_BASE" $DB_BASE_PATCHES || return $?
    logmsg_debug "Database schema should have been initialized at this point: core schema file only"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
)

loaddb_sampledata() (
    _DB_TXT="loading default sample data"
    _DB_TAG="loaddb_sampledata"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA" && return $?

    loaddb_initial && \
    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----" && \
    loaddb_list "$DB_DATA" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: sample datacenter for tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
)

loaddb_default() (
    _DB_TXT="loading default REST API test data"
    _DB_TAG="loaddb_default"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA" "$DB_DATA_TESTREST" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} -------"
    loaddb_sampledata && \
    loaddb_list "$DB_DATA_TESTREST" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for common REST API tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
)

loaddb_topo_loc() {
    _DB_TXT="raw-topo-location"
    _DB_TAG="loaddb_topo_loc"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_TOPOL" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_TOPOL" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for raw topology-location tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_topo_pow() {
    _DB_TXT="raw-topo-power"
    _DB_TAG="loaddb_topo_pow"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_TOPOP" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_TOPOP" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for raw topology-power tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_sampledata_topo_loc() {
    _DB_TXT="sampledata-topo-location"
    _DB_TAG="loaddb_sampledata_topo_loc"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA" "$DB_DATA_TESTREST" "$DB_TOPOL" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_sampledata && \
    loaddb_list "$DB_TOPOL" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for topology-location tests with sample data"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_sampledata_topo_pow() {
    _DB_TXT="sampledata-topo-power"
    _DB_TAG="loaddb_sampledata_topo_pow"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA" "$DB_DATA_TESTREST" "$DB_TOPOP" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_sampledata && \
    loaddb_list "$DB_TOPOP" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for topology-power tests with sample data"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_rack_power() {
    _DB_TXT="rack-power"
    _DB_TAG="loaddb_rack_power"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_RACK_POWER" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_RACK_POWER" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for rack-power tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_dc_power_UC1() {
    _DB_TXT="dc-power-UC1"
    _DB_TAG="loaddb_dc_power_UC1"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DC_POWER_UC1" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_DC_POWER_UC1" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for dc-power-UC1 tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_dc_power() {
    _DB_TXT="dc-power"
    _DB_TAG="loaddb_dc_power"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DC_POWER" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_DC_POWER" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for dc-power tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_averages() {
    _DB_TXT="averages"
    _DB_TAG="loaddb_averages"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA" "$DB_DATA_TESTREST" "$DB_AVERAGES" "$DB_AVERAGES_RELATIVE" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_sampledata && \
    loaddb_list "$DB_AVERAGES" "$DB_AVERAGES_RELATIVE" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for averages tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

loaddb_current() {
    _DB_TXT="current"
    _DB_TAG="loaddb_current"
    tarballdb_fastload "${_DB_TXT}" "${_DB_TAG}" "$DB_BASE" $DB_BASE_PATCHES "$DB_DATA_CURRENT" && return $?

    echo "CI-TESTLIB_DB - reset db: ${_DB_TXT} ----------"
    loaddb_initial && \
    loaddb_list "$DB_DATA_CURRENT" || return $?
    logmsg_debug "Database schema and data should have been initialized at this point: for current tests"

    tarballdb_fastsave "${_DB_TAG}"
    return $?
}

accept_license_WRAPPER() {
    # Some scripts only care about database and do not have weblib.sh included
    # Also some scripts know their webserver is not yet started ;)
    [ x"${ACCEPT_LICENSE-}" = xno ] || \
    if type -t accept_license | grep -q 'function' ; then
        accept_license
        return $?
    else
        echo "CI-TESTLIB_DB - reset db: accept_license() not applicable to this script"
    fi
    return 0
}

reloaddb_init_script_WRAPPER() {
    # Prepare sandbox for the test: ensure the database is freshly made
    # and licenses to not interfere; the accept_license() routine is
    # defined in weblib.sh at the moment.
    # As parameter(s) pass the loaddb_*() routine names to execute
    # while the database is down.
    if reloaddb_stops_BIOS && \
        [ -x "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" ] \
    ; then
        echo "CI-TESTLIB_DB - reset db: stop 42ity ---------------"
        "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" --stop
    fi

    while [ $# -gt 0 ]; do
        $1 || return $?
        shift
    done

    if reloaddb_stops_BIOS && \
        [ -x "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" ] \
    ; then
        echo "CI-TESTLIB_DB - reset db: start 42ity ---------------"
        "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" --start-quick || return $?
    fi

    accept_license_WRAPPER
    return $?
}

init_script_wipedb(){
    # This routine just wipes the database (twice) with agents running
    # This requires the database schema initialized and mysql running
    logmsg_debug "`wipedb 2>&1`"
    # Retry just in case some running agent barged in to add data
    wipedb || { logmsg_error "Could not wipe DB"; return 1; }
    accept_license_WRAPPER
    return $?
}

init_script_wipedb_restart(){
    # This routine stops 42ity services that rely on DB, wipes DB and restarts
    # This requires the database schema initialized and mysql running
    if reloaddb_stops_BIOS ; then
        echo "CI-TESTLIB_DB - reset db: stop BIOS-DB-INIT -------"
        sut_run "systemctl stop bios-db-init" || \
        sut_run "systemctl stop bios-db-init" || \
        logmsg_error "Can not stop bios-db-init (and depending services)"
    fi

    wipedb || { logmsg_error "Could not wipe DB"; return 1; }

    if reloaddb_stops_BIOS ; then
        echo "CI-TESTLIB_DB - reset db: start BIOS-DB-INIT -------"
        if isRemoteSUT ; then
            sut_run "systemctl start bios-db-init" && \
            sut_run "systemctl start tntnet@bios" || \
            { logmsg_error "Can not start bios-db-init (and depending services)"; return 1; }
        fi

        if [ -x "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" ] ; then
            echo "CI-TESTLIB_DB - reset db: start BIOS ---------------"
            "$CHECKOUTDIR/tests/CI/ci-rc-bios.sh" --start-quick || return $?
        fi
    fi

    accept_license_WRAPPER
    return $?
}

init_script_initial(){
    reloaddb_init_script_WRAPPER loaddb_initial
}

init_script_sampledata(){
    reloaddb_init_script_WRAPPER loaddb_sampledata
}

init_script_default(){
    reloaddb_init_script_WRAPPER loaddb_default
}

init_script(){
    # Alias, legacy
    init_script_default "$@"
}

init_script_topo_loc(){
    reloaddb_init_script_WRAPPER loaddb_topo_loc
}

init_script_topo_pow(){
    reloaddb_init_script_WRAPPER loaddb_topo_pow
}

init_script_sampledata_topo_loc(){
    reloaddb_init_script_WRAPPER loaddb_sampledata_topo_loc
}

init_script_sampledata_topo_pow(){
    reloaddb_init_script_WRAPPER loaddb_sampledata_topo_pow
}

init_script_rack_power(){
    reloaddb_init_script_WRAPPER loaddb_rack_power
}

init_script_averages(){
    reloaddb_init_script_WRAPPER loaddb_averages
}

init_script_current(){
    reloaddb_init_script_WRAPPER loaddb_current
}

init_script_dc_power_UC1(){
    reloaddb_init_script_WRAPPER loaddb_dc_power_UC1
}

init_script_dc_power(){
    reloaddb_init_script_WRAPPER loaddb_dc_power
}

:
