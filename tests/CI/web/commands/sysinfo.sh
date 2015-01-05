# Check getting server system info (TODO: authorized only?)
test_it "sysinfo_get"
SYSINFO="`api_get_content '/admin/sysinfo'`"
RES=$?
echo "=== SYSINFO:" >&2
echo "$SYSINFO" >&2
print_result $RES

test_it "sysinfo_parsable"
SYSINFO_PARSED="`echo "$SYSINFO" | $CHECKOUTDIR/tools/JSON.sh -b`"
RES=$?
echo "=== SYSINFO_PARSED:" >&2
echo "$SYSINFO_PARSED" >&2
[ $RES = 0 -a -n "$SYSINFO_PARSED" ]
print_result $?

getval() {
    echo "$SYSINFO_PARSED" | egrep "$1" | \
    while read _C _T; do echo "$_T"; done | \
    sed 's,^"*,,' | sed 's,"*$,,'
}

test_it "sysinfo_runs_in_container"
RES=0
SYSINFO_CONTAINER_TYPE="`getval '^\["server-os-features","virt","container","type"\]'`" || RES=$?
SYSINFO_CONTAINER_FLAG="`getval '^\["server-os-features","virt","container","flag"\]'`" || RES=$?
echo "=== SYSINFO_CONTAINER_TYPE: '$SYSINFO_CONTAINER_TYPE' (code '$SYSINFO_CONTAINER_FLAG')" >&2
[ $RES = 0 -a -n "$SYSINFO_CONTAINER_TYPE" -a \
    x"$SYSINFO_CONTAINER_TYPE" != x'""' ]
print_result $?

test_it "sysinfo_runs_in_virtmachine"
RES=0
SYSINFO_VIRTMACHINE_TYPE="`getval '^\["server-os-features","virt","virtmachine","type"\]'`" || RES=$?
SYSINFO_VIRTMACHINE_FLAG="`getval '^\["server-os-features","virt","virtmachine","flag"\]'`" || RES=$?
echo "=== SYSINFO_VIRTMACHINE_TYPE: '$SYSINFO_VIRTMACHINE_TYPE' (code '$SYSINFO_VIRTMACHINE_FLAG')" >&2
[ $RES = 0 -a -n "$SYSINFO_VIRTMACHINE_TYPE" -a \
    x"$SYSINFO_VIRTMACHINE_TYPE" != x'""' ]
print_result $?

if [ "$TEST_RETURN_CONTAINER_FLAG" = yes ]; then
	[ "$SYSINFO_CONTAINER_FLAG" -gt 255 ] && \
		SYSINFO_CONTAINER_FLAG="`echo $SYSINFO_CONTAINER_FLAG % 255 | bc`"
	exit $SYSINFO_CONTAINER_FLAG
fi

if [ "$TEST_RETURN_VIRTMACHINE_FLAG" = yes ]; then
	[ "$SYSINFO_VIRTMACHINE_FLAG" -gt 255 ] && \
		SYSINFO_VIRTMACHINE_FLAG="`echo $SYSINFO_VIRTMACHINE_FLAG % 255 | bc`"
	exit $SYSINFO_VIRTMACHINE_FLAG
fi

