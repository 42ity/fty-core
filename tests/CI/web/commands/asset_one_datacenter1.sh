test_it
curlfail_push_expect_404
api_get_json /asset/datacenter/2 >&5
print_result $?
curlfail_pop
