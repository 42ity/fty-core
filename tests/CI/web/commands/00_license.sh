sut_run "rm -f /var/lib/bios/license $CHECKOUTDIR/var/bios/license" || true

test_it "license_status_not_ok"
api_get_json '/admin/license/status' >&5
print_result $?

test_it "license_acceptance"
api_auth_post_content '/admin/license' "foobar" >&5
print_result $?

test_it "license_status_ok"
api_get_json '/admin/license/status' | sed 's|\(accepted_at":"\)[0-9]*"|\1XXX"|' >&5
print_result $?
