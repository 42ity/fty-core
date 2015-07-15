#TEST for /ui/properties GET/PUT

curlfail_push "warn" ""
test_it "GET_ui_properties_1"
api_get /ui/properties | grep -q '{"key1" : "value1", "key2" : "value2"'
TESTLIB_QUICKFAIL=no print_result $?
curlfail_pop

test_it "PUT_ui_properties_1"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value42"}'
print_result $?

test_it "GET_ui_properties_2"
api_get /ui/properties | grep -q '{"key1" : "value1", "key2" : "value42"'
print_result $?

test_it "PUT_ui_properties_2"
api_auth_put /ui/properties '{"key1" : "value1", "key2" : "value2"}'
print_result $?

#TODO: can't validate JSON via cxxtools
#test_it "PUT_invalid_JSON"
#api_auth_put /usr/properties
#print_result $?
