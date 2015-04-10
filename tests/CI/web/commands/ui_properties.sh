#TEST for /ui/properties GET/PUT

test_it "GET_ui_properties_1"
api_get /ui/properties | grep -q '{"key1" : "value1", "key2" : "value2"'
WEBLIB_QUICKFAIL=no print_result $?

test_it "PUT_ui_properties_1"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value42"}' | \
    grep -q 'HTTP/1.1 200 OK'
print_result $?

test_it "GET_ui_properties_2"
api_get /ui/properties | grep -q '{"key1" : "value1", "key2" : "value42"'
print_result $?

test_it "PUT_ui_properties_2"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value2"}' | \
    grep -q 'HTTP/1.1 200 OK'
print_result $?

#TODO: can't validate JSON via cxxtools
#test_it "PUT_invalid_JSON"
#api_auth_put /usr/properties
#print_result $?
