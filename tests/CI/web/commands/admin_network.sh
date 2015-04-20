#TEST for /admin/network POST/DELETE
#TODO(?): can't run twice, needs to extract ids from POSTed entries

curlfail_push_expect_404
do_test_match "admin_network_bad_method" api_get /admin/network \
    'HTTP/1.1 404 Not Found'
print_result $?

do_test_match "admin_network_bad_request" api_auth_post /admin/network \
    '{"net" : "123"}' \
    'HTTP/1.1 400 Bad Request'
print_result $?
curlfail_pop



curlfail_expect_noerrors
do_test_match "admin_network_post_excl1" api_auth_post /admin/network \
    '{"net" : "10.20.30.40/24", "status" : "E"}' \
    '{ "id" : "5" }'
print_result $?

# check the double POST just returns the same ID
do_test_match "admin_network_post_excl2" api_auth_post /admin/network \
    '{"net" : "10.20.30.40/24", "status" : "E"}' \
    '{ "id" : "5" }'
print_result $?

# ... but manual status is different
do_test_match "admin_network_post_man1" api_auth_post /admin/network \
    '{"net" : "10.20.30.40/24", "status" : "M"}' \
    '{ "id" : "6" }'
print_result $?

do_test_match "admin_network_post_man2" api_auth_post /admin/network \
    '{"net" : "10.20.30.40/24", "status" : "M"}' \
    '{ "id" : "6" }'
print_result $?

# and let's del 'em all
do_test_match "admin_network_delete5" \
    api_auth_delete /admin/network/5 \
    'HTTP/1.1 200 OK'
print_result $?

do_test_match "admin_network_delete6" \
    api_auth_delete /admin/network/6 \
    'HTTP/1.1 200 OK'
print_result $?
curlfail_pop



curlfail_push_expect_400
# delete non-existent network
do_test_match "admin_network_delete12" \
    api_auth_delete /admin/network/12 \
    'HTTP/1.1 400 Bad Request'
print_result $?

# delete automatic network
test_it "admin_network_delete7" \
    api_auth_delete /admin/network/7 \
    'HTTP/1.1 400 Bad Request'
print_result $?
curlfail_pop



curlfail_push_expect_404
# non numeric id in url - catched by tntnet.xml mapping
do_test_match "admin_network_deleteabcd" \
    api_auth_delete /admin/network/abcd \
    'HTTP/1.1 404 Not Found'
print_result $?
curlfail_pop
