test_it
api_get_json /topology/location?from=7026\&recursive=yes\&filter=rows >&5
print_result $?
