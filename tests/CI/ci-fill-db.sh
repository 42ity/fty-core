#!/bin/bash
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
#! \file   ci-fill-db.sh
#  \brief  tests database files import
#  \author Barbora Stepankova <BarboraStepankova@Eaton.com>
#  \author Tomas Halman <TomasHalman@Eaton.com>

# Include our standard routines for CI scripts
. "`dirname $0`"/scriptlib.sh || \
    { echo "CI-FATAL: $0: Can not include script library" >&2; exit 1; }
NEED_BUILDSUBDIR=no NEED_CHECKOUTDIR=yes determineDirs_default || true
. "`dirname $0`"/testlib.sh || die "Can not include common test script library"
. "`dirname $0`"/testlib-db.sh || die "Can not include database test script library"
cd "$CHECKOUTDIR" || die "Unusable CHECKOUTDIR='$CHECKOUTDIR'"
logmsg_info "Using CHECKOUTDIR='$CHECKOUTDIR' to test the database initialization"
[ -d "$DB_LOADDIR" ] || die "Unusable DB_LOADDIR='$DB_LOADDIR' or testlib-db.sh not loaded"

# Note: this default log filename will be ignored if already set by caller
init_summarizeTestlibResults "${BUILDSUBDIR}/tests/CI/web/log/`basename "${_SCRIPT_NAME}" .sh`.log" ""
settraps 'exit_summarizeTestlibResults'

set -u
#set -e
#set -x

# Not wrapped with testlib-db init_script* routines - this test is often
# a quick-run component of tests that are in control, or via the CI suite
test_it "loaddb_initial+loaddb_sampledata"
loaddb_sampledata       # in testlib-db this includes loaddb_initial()
print_result $?

test_it "presence of t_bios_asset_element_type"
do_select "select * from t_bios_asset_element_type"
print_result $?

# The trap-handler should display the summary (if any)
exit
