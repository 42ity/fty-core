#!/bin/sh
test_it "metric/computed/average"

# An OK request
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 200 OK"
print_result $?

# end_ts can be empty
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 200 OK"
print_result $?

# type can be empty
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 200 OK"
print_result $?

# both type and end_ts can be empty
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 200 OK"
print_result $?

# Root not Object type
api_post_json_cmp "/metric/computed/average" \
   '[
        "start_ts",
        "end_ts",
        "step",
        "type",
        {
            "5" : [ "a.b" ]
        }
    ]' \
    "HTTP/1.1 400 Bad Request"
print_result $?

api_post_json_cmp "/metric/computed/average" \
    '"start_ts" : "end_ts"' \
    "HTTP/1.1 400 Bad Request"
print_result $?

api_post_json_cmp "/metric/computed/average" '' "HTTP/1.1 400 Bad Request"
print_result $?

api_post_json_cmp "/metric/computed/average" \
    '{ sdfsd fs  sdfsrf sdf fsfesfefs dfsd }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

api_post_json_cmp "/metric/computed/average" \
    ' sdfsd f sdf : fsdf" " s  sdfsrf sdf fsfesfefs dfsd }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# Bad step
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "6m",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# Wrong type
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmeticwfwr rwan",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "zelene koule frantu pepika",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts end_ts
# Both Z's missing
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000",
        "end_ts" : "20150102000000",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# end_ts Z missing
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts Z missing
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts one number less
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "2015010100000",
        "end_ts" : "20150102000000",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# end_ts one number less
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "2015010200000",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# end_ts negative value  
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "2015010100000Z",
        "end_ts" : "-1",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts negative value
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "-201501010000",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts nonsensical value 
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "daewoZ",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# end_ts nonsensical value 
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150102000000Z",
        "end_ts" : "sdf sdf sd",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# start_ts bad value
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101998877Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# end_ts bad value
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102998877Z",
        "step" : "8h",
        "type" : "",
        "sources" :
        {
            "5" : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# TODO end_ts < start_ts

# sources empty
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# Bad sources
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            5 : [ "a.b" ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# 
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" :  "a.b" 
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# 
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" :  []
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# 
api_post_json_cmp "/metric/computed/average" \
   '{
        "start_ts" : "20150101000000Z",
        "end_ts" : "20150102000000Z",
        "step" : "8h",
        "type" : "arithmetic_mean",
        "sources" :
        {
            "5" :  [ "asd.das",  adsads ]
        }
    }' \
    "HTTP/1.1 400 Bad Request"
print_result $?

# TODO: more source tests
