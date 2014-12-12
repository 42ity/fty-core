test_it "topology/power bad_input 1"

[ "`api_get "/topology/power?from=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?from=21474836470000000000000000" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?from=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?from=asd54" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?from=%20+%20" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=x" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=21474836470000000000000000" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=asd54" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=%20+%20" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?



[ "`api_get "/topology/power?to=x&from=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?to=x&filter_dc=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?
[ "`api_get "/topology/power?to=x&filter_group=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?


[ "`api_get "/topology/power?from=x&filter_dc=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?
[ "`api_get "/topology/power?from=x&filter_group=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?


[ "`api_get "/topology/power?from=x&to=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?

[ "`api_get "/topology/power?filter_dc=x&to=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?
[ "`api_get "/topology/power?filter_group=x&to=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?


[ "`api_get "/topology/power?filter_dc=x&from=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?
[ "`api_get "/topology/power?filter_group=x&from=y" | \
            grep "HTTP/1.1 400 Bad Request"`" ]
print_result $?


