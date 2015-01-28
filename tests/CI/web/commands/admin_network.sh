#!/bin/sh

#TEST for /admin/network POST/DELETE
#TODO: can't run twice, needs to extract ids from POSTed entries

test_it "admin_network_bad_method"
api_get /admin/network | grep -q 'HTTP/1.1 404 Not Found'
print_result $?

test_it "admin_network_bad_request"
api_auth_post /admin/network '{"net" : "123"}' | grep -q 'HTTP/1.1 400 Bad Request'
print_result $?

test_it "admin_network_post_excl1"
api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "E"}' | grep -q '{ "id" : "5" }'
print_result $?

# check the double POST just returns the same ID
test_it "admin_network_post_excl2"
api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "E"}' | grep -q '{ "id" : "5" }'
print_result $?

# ... but manual status is different
test_it "admin_network_post_man1"
api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "M"}' | grep -q '{ "id" : "6" }'
print_result $?

test_it "admin_network_post_man2"
api_auth_post /admin/network '{"net" : "10.20.30.40/24", "status" : "M"}' | grep -q '{ "id" : "6" }'
print_result $?

# and let's del 'em all
test_it "admin_network_delete11"
api_auth_delete /admin/network/5 &> curl.stdout
if ! grep -q 'HTTP/1.1 200 OK' curl.stdout; then
    cat curl.stdout
    print_result 1
else
    print_result 0
fi

# and let's del 'em all
test_it "admin_network_delete12"
api_auth_delete /admin/network/6 &> curl.stdout
if ! grep -q 'HTTP/1.1 200 OK' curl.stdout; then
    cat curl.stdout
    print_result 1
else
    print_result 0
fi

# delete non-existent network
test_it "admin_network_delete3"
api_auth_delete /admin/network/12 &> curl.stdout
if ! grep -q 'HTTP/1.1 400 Bad Request' curl.stdout; then
    cat curl.stdout
    print_result 1
else
    print_result 0
fi

# non numeric id in url - catched by tntnet.xml mapping
test_it "admin_network_delete4"
api_auth_delete /admin/network/abcd &> curl.stdout
if ! grep -q 'HTTP/1.1 404 Not Found' curl.stdout; then
    cat curl.stdout
    print_result 1
else
    print_result 0
fi

# delete automatic network
test_it "admin_network_delete5"
api_auth_delete /admin/network/7 &> curl.stdout
if ! grep -q 'HTTP/1.1 400 Bad Request' curl.stdout; then
    cat curl.stdout
    print_result 1
else
    print_result 0
fi
