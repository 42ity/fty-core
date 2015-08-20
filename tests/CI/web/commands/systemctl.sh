# PoCv2  for test /admin/systemctl

# print the systemctl show in format of REST API call to help the comparing
show2json() {
	# $1 = systemd service name
	sudo systemctl show -p ActiveState -p SubState -p LoadState -p UnitFileState "$1" | \
	(	echo "{\"$1\": {"
		N=0
		while read LINE; do 
			[ "$N" = 0 ] && N=1 || echo ","
			echo "$LINE" | sed 's/^\([^=]*\)=\([^=]*\)$/    "\1": "\2"/'
		done
		echo "} }"
	) \
	#| "$JSONSH" -N
}

test_it "systemctl_status_malamute"

OUTREST="`api_auth_get_json /admin/systemctl/status/malamute`"
RESREST=$?
OUTSYSD="`show2json malamute`"
RESSYSD=$?

[ "$RESREST" = 0 -a "$RESSYSD" = 0 ] && \
[ -n "$CMPJSON_SH" ] && [ -x "$CMPJSON_SH" ] && \
    "$CMPJSON_SH" -s "$OUTREST" "$OUTSYSD"

print_result $?

