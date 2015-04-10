test_it dev=6
api_get_json '/metric/current?dev=6' >&5
print_result $?

test_it dev=7
api_get_json '/metric/current?dev=7' >&5
print_result $?

test_it dev=3
api_get_json '/metric/current?dev=3' >&5
print_result $?

test_it default
api_get_json '/metric/current' >&5
print_result $?
