test_it "topology/location bad_input 1"
[ "`api_get "/topology/location?from=api/v1/asset/datacenter/5&to=api/v1/asset/root/53" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=&to=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from&to" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&recursive=no" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&recursive=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&filter=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&filter=rooms" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?to=api/v1/asset/group/1&filter=rows&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/ass%20et/row/1234" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/data+center/1234" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/room/1234&filter=datacenters" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?


[ "`api_get "/topology/location?from=api/v1/asset/room/1234&filter=adys" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/room/1234&recursive=s" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/room/1234&filter=groups&recursive=ysfd" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/room/1234&filter=datacenters&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/location?from=api/v1/asset/room/1234&filter=asd&recursive=yes" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?
# More tests to come













