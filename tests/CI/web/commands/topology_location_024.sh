test_it
api_get_json /topology/location?from=7000\&recursive=yes\&filter=groups >&5
print_result $?
