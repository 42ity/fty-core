test_it
api_get_json /topology/location?from=7032\&recursive=yes >&5
print_result $?
