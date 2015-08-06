test_it
api_get_json /topology/location?from=7000 >&5
print_result $?
