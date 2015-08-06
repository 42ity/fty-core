test_it dev=6
api_get_json '/metric/current?dev=6' >&5
print_result $?

test_it dev=7
api_get_json '/metric/current?dev=7' >&5
print_result $?

test_it dev=3
curlfail_push_expect_404
api_get_json '/metric/current?dev=3' >&5
print_result $?
curlfail_pop

test_it default
curlfail_push_expect_400
api_get_json '/metric/current' >&5
print_result $?
curlfail_pop
