#!/bin/bash

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
#! \file    ci-test-db.sh
#  \brief   tests database setup
#  \author  Barbora Stepankova <BarboraStepankova@Eaton.com>
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \author  Alena Chernikava <AlenaChernikava@Eaton.com>
#  \author  Jim Klimov <EvgenyKlimov@Eaton.com>
#  \details Every database test has its own requirements regarding input data.
#       That is why before every database test database should be initialised
#       and appropriate testing data should be filled.
#
# Requirements:
#   - mariadb running
#   - db user root without password
#

# Use the test_it() framework
TESTLIB_COUNT_PASS=0
TESTLIB_COUNT_SKIP=0
TESTLIB_COUNT_FAIL=0
TESTLIB_COUNT_TOTAL=0
TESTLIB_LIST_FAILED=""
TESTLIB_LIST_FAILED_IGNORED=""
TESTLIB_LIST_PASSED=""

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no NEED_CHECKOUTDIR=yes determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to build the database tests"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"
[ -d "$CSV_LOADDIR_BAM" ] || die "Unusable CSV_LOADDIR_BAM='$CSV_LOADDIR_BAM'"

set -u
#set -e

init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log"
settraps 'exit_summarizeTestlibResults'

echo "-------- ensure bins to test are up to date -------"
test_it "tested_source_is_configured"
[ -n "$BUILDSUBDIR" ] && [ -x "$BUILDSUBDIR/config.status" ] || \
    ./autogen.sh --install-dir / --configure-flags \
        "--prefix=$HOME --with-saslauthd-mux=/var/run/saslauthd/mux" \
    ${AUTOGEN_ACTION_CONFIG}
print_result $?

test_it "tested_source_is_built"
./autogen.sh --optseqmake --nodistclean ${AUTOGEN_ACTION_MAKE} \
    test-db2 test-db-asset-crud test-dbtopology test-totalpower
print_result $?

sleep 1

# From here on we use the build directory since libtool-generated
# scripts which wrap our build products may want that
NEED_BUILDSUBDIR=yes determineDirs_default || true
cd "$BUILDSUBDIR" || die "Unusable BUILDSUBDIR='$BUILDSUBDIR'"
logmsg_info "Using BUILDSUBDIR='$BUILDSUBDIR' to run the database tests"

# Tests below have their failsafes
set +e

echo "-------------------- test-db2 --------------------"
test_it "db_prepare_sample_data"
loaddb_sampledata
CITEST_QUICKFAIL=yes print_result $? "Can not prepare the database"

test_it test-db2
"$BUILDSUBDIR"/test-db2
print_result $?
sleep 1

echo "-------------------- test-db-asset-crud-----"
test_it "db_prepare_crud"
loaddb_initial \
&& loaddb_file "$DB_CRUD"
CITEST_QUICKFAIL=yes print_result $? "Can not prepare the database"

test_it test-db-asset-crud
"$BUILDSUBDIR"/test-db-asset-crud
print_result $?
sleep 1

for T in location power ; do
    case "$T" in
        power) P="$DB_TOPOP" ; F=loaddb_topo_pow ;;
        location) P="$DB_TOPOL" ; F=loaddb_topo_loc ;;
        *) continue ;;
    esac
    echo "-------------------- test-dbtopology $T --------------------"
    test_it "db_prepare_topology_$T"
    eval $F
    CITEST_QUICKFAIL=yes print_result $? "Can not prepare the database"

    test_it "test-dbtopology::$T"
    "$BUILDSUBDIR"/test-dbtopology "[`basename "$P"`]"
    print_result $?
    sleep 1
done

for T in rack dc ; do
    case "$T" in
        rack) P="$DB_RACK_POWER" ; F=loaddb_rack_power ;;
        dc) P="$DB_DC_POWER" ; F=loaddb_dc_power ;;
        *) continue ;;
    esac
    echo "-------------------- test-total-power for $T ---------------"
    test_it "db_prepare_totalpower_$T"
    eval $F
    CITEST_QUICKFAIL=yes print_result $? "Can not prepare the database"

    test_it "test-totalpower::$T"
    "$BUILDSUBDIR"/test-totalpower "[`basename "$P"`]"
    print_result $?
    sleep 1
done


# The trap-handler should display the summary (if any)
exit
