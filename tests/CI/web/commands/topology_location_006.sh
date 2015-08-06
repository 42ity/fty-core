test_it
api_get_json /topology/location?from=7004\&recursive=no\&filter=devices >&5
print_result $?
