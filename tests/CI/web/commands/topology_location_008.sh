test_it
api_get_json /topology/location?from=7000\&filter=racks\&recursive=yes >&5
print_result $?
