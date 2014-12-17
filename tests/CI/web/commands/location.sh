test_it "topology/location bad_input 1"

[ "`api_get "/topology/location?from=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=21474836470000000000000000" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=asd54" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=%20+%20" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=21474836470000000000000000" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=asd54" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=%20+%20" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=5&to=53" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=&to=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from&to" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=1&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=1&recursive=no" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=41&recursive=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=0&filter=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=1&filter=rooms" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=1&filter=rows&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1234&filter=datacenters" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=4321&filter=adys" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=4321&filter=roo%20ms" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=4321&filter=row+s" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1111&recursive=s" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=5&filter=groups&recursive=ysfd" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1234&filter=datacenters&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1234&filter=datacenters&recursive=no" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1234&filter=asd&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=1234&filter=d&recursive=no" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

# More tests to come













