test_it "topology/location__bad_input__1"
[ "`api_get '/topology/location?from=x' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__2"
[ "`api_get '/topology/location?from=21474836470000000000000000' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__3"
[ "`api_get '/topology/location?from=' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__4"
[ "`api_get '/topology/location?from=asd54' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__5"
[ "`api_get '/topology/location?from=%20+%20' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__6"
[ "`api_get '/topology/location?to=x' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__7"
[ "`api_get '/topology/location?to=21474836470000000000000000' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__8"
[ "`api_get '/topology/location?to=' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__9"
[ "`api_get '/topology/location?to=asd54' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__10"
[ "`api_get '/topology/location?to=%20+%20' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__11"
[ "`api_get '/topology/location?from=5&to=53' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__12"
[ "`api_get '/topology/location?from=&to=' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__13"
[ "`api_get '/topology/location?from&to' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__14"
[ "`api_get '/topology/location?to=1&recursive=yes' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__15"
[ "`api_get '/topology/location?to=1&recursive=no' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__16"
[ "`api_get '/topology/location?to=41&recursive=x' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__17"
[ "`api_get '/topology/location?to=0&filter=x' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__18"
[ "`api_get '/topology/location?to=1&filter=rooms' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__19"
[ "`api_get '/topology/location?to=1&filter=rows&recursive=yes' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__20"
[ "`api_get '/topology/location?from=1234&filter=datacenters' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__21"
[ "`api_get '/topology/location?from=4321&filter=adys' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__22"
[ "`api_get '/topology/location?from=4321&filter=roo%20ms' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__23"
[ "`api_get '/topology/location?from=4321&filter=row+s' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__24"
[ "`api_get '/topology/location?from=1111&recursive=s' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__25"
[ "`api_get '/topology/location?from=5&filter=groups&recursive=ysfd' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__26"
[ "`api_get '/topology/location?from=1234&filter=datacenters&recursive=yes' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__27"
[ "`api_get '/topology/location?from=1234&filter=datacenters&recursive=no' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__28"
[ "`api_get '/topology/location?from=1234&filter=asd&recursive=yes' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__29"
[ "`api_get '/topology/location?from=1234&filter=d&recursive=no' | \
            grep 'HTTP/1.1 400 Bad Request'`" ]
print_result $?

test_it "topology/location__bad_input__30"
[ "`api_get '/topology/location?from=5019' | \
            grep 'HTTP/1.1 404 Not Found'`" ]
print_result $?

