#!/bin/sh
test_it "admin_network_bad_method"
api_get /admin/network | grep -q 'HTTP/1.1 405 Method Not Allowed'
print_result $?

test_it "admin_network_bad_request"
api_auth_post /admin/network | grep -q 'HTTP/1.1 400 Bad Request'
print_result $?

test_it "admin_network_post"
