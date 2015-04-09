test_it
api_get_json /topology/location?from=7000\&filter=devices >&5
print_result $?
