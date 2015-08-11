do_test_match "ifaces_get" api_get_json '/admin/ifaces' '.*ifaces.*lo.*'
print_result $?

test_it "iface_lo_get"
api_get_json '/admin/iface/lo' >&5
print_result $?

test_it "iface_foobar_get"
curlfail_push_expect_404
api_get_json '/admin/iface/foobar' >&5
print_result $?
curlfail_pop
