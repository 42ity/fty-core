###############################################################
[ -z "$JSONSH" -o ! -x "$JSONSH" ] && JSONSH="$CHECKOUTDIR/tools/JSON.sh"

### Verify that logins are authorized for both tokens with a plus and without
testLoginSysinfo() {
    ### A chain of tests over SYSINFO_T value with inherited RES value
    ### Both are set by caller
    ### Returns success (0) or failure (non-0)
    RES2=0
    [ $RES = 0 -a -n "$SYSINFO_T" ] || RES2=$?
    [ "$RES2" != 0 ] && \
	echo "=== Not good SYSINFO_T ($RES):" && echo "$SYSINFO_T" && \
	return $RES

    SYSINFO_PARSED="`echo "$SYSINFO_T" | ${JSONSH} -l --no-newline`"
    RES=$?
    [ $RES = 0 -a -n "$SYSINFO_PARSED" ] || RES2=$?
    [ "$RES2" != 0 ] && \
	echo "=== Not good JSON ($RES):" && echo "$SYSINFO_PARSED" && \
	echo "=== SYSINFO_T:" && echo "$SYSINFO_T" && \
	return $RES

    JPATH='"operating-system","uname","version"'
    SYSINFO_PARSED="`echo "$SYSINFO_T" | ${JSONSH} -x "$JPATH" | grep -v unauthorized`"
    RES=$?
    [ $RES = 0 -a -n "$SYSINFO_PARSED" ] || RES2=$?
    [ "$RES2" != 0 ] && \
	echo "=== Got 'unauthorized' ($RES):" && echo "$SYSINFO_PARSED" && \
	echo "=== SYSINFO_T:" && echo "$SYSINFO_T" && \
	return $RES

    return 0
}

test_it "_api_get_token_withoutplus"
_TOKEN_=""
TOKEN="`_api_get_token_withoutplus`"
RES=$?
[ $RES != 0 ] && TOKEN="___xxx___"
_TOKEN_="$TOKEN"
print_result $RES

echo "=== Verify various login methods with TOKEN='$TOKEN'"

test_it "sysinfo_get_wHeader_noplus_auth=2"
SYSINFO_T="`api_auth_get_content '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_post_wHeader_noplus_auth=2"
SYSINFO_T="`api_auth_post_content '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_get_wToken_noplus_auth=2"
SYSINFO_T="`api_auth_get_content_wToken '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_post_wToken_noplus_auth=2"
SYSINFO_T="`api_auth_post_content_wToken '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?



test_it "_api_get_token_withplus"
_TOKEN_=""
TOKEN="`_api_get_token_withplus`"
RES=$?
[ $RES != 0 ] && TOKEN="___xxx___"
_TOKEN_="$TOKEN"
print_result $RES

echo "=== Verify various login methods with TOKEN='$TOKEN'"

test_it "sysinfo_get_wHeader_withplus_auth=2"
SYSINFO_T="`api_auth_get_content '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_post_wHeader_withplus_auth=2"
SYSINFO_T="`api_auth_post_content '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_get_wToken_withplus_auth=2"
SYSINFO_T="`api_auth_get_content_wToken '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

test_it "sysinfo_post_wToken_withplus_auth=2"
SYSINFO_T="`api_auth_post_content_wToken '/admin/sysinfo'`"; RES=$?
testLoginSysinfo; print_result $?

### Reset the _TOKEN_ so other tests can re-login again
_TOKEN_=""
