
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


#! \file time.sh
#  \author Jim Klimov <EvgenyKlimov@Eaton.com>
#  \author Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \brief Not yet documented file

echo "***************************************************************************************************"
echo "********* Prerequisites ***************************************************************************"
echo "***************************************************************************************************"
init_script

[ -z "${JSONSH-}" ] && \
    for F in "$CHECKOUTDIR/tools/JSON.sh" "$SCRIPTDIR/JSON.sh"; do
        [ -x "$F" -a -s "$F" ] && JSONSH="$F" && break
    done

SED_FILTER_TIME='s|.*\"time\"[[:blank:]]*:[[:blank:]]*\"\([^\"]*\)\".*|\1|p'
ZEROEPOCH='{ "time":"1970-01-01T00:00:00Z" }'

# Check getting time
test_it "time_get"
TIME="`api_get '/admin/time' | sed -n "$SED_FILTER_TIME"`"
TIME_S_GET="`date --utc -d"$TIME" +%s 2> /dev/null`"
TIME_S_NOW="`date --utc +%s`"
[ "`expr "$TIME_S_NOW" - "$TIME_S_GET"`" -lt 10 ]
print_result $?

# Check setting time as unprivileged user
test_it "unauth_time_set"
curlfail_push_expect_401
api_post '/admin/time' "$ZEROEPOCH" >/dev/null
print_result $?
curlfail_pop

# Check setting time as privileged user
# NOTE: Doesn't work within lxc
SYSINFO="`api_get_json '/admin/sysinfo'`"
RES=$?
JPATH='"operating-system","container"'
SYSINFO_CONTAINER="`echo "$SYSINFO" | ${JSONSH} -x="$JPATH"`" || RES=$?
if [ $RES = 0 -a -n "$SYSINFO_CONTAINER" -a \
     x"$SYSINFO_CONTAINER" != x'""' ] && \
    echo "$SYSINFO_CONTAINER" | egrep 'lxc' >/dev/null ; \
then
    echo "SKIPPED test auth_time_set because server runs in a container" >&2
    echo "=== SYSINFO_CONTAINER ($RES): '$SYSINFO_CONTAINER'" >&2
else
    test_it "auth_time_set"
    TIME_NOW="`date --utc +%FT%TZ`"
    TIME="`api_auth_post '/admin/time' "$ZEROEPOCH" | \
	sed -n "$SED_FILTER_TIME"`"
    api_auth_post '/admin/time' '{ "time":"'"$TIME_NOW"'" }' > /dev/null
    TIME_S="`date -d"$TIME" +%s 2> /dev/null`"
    [ "$TIME_S" ] && [ "$TIME_S" -lt 10 ]
    print_result $?
fi

# Check setting nonsense
test_it "wrong_time"
curlfail_push_expect_400
api_auth_post '/admin/time' 'stardate 48960.9' >/dev/null
print_result $?
curlfail_pop
