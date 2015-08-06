test_it
RES=0
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=power | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=avg_power_last_day,avg_power_last_week | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=10,19\&arg2=avg_humidity_last_day,avg_humidity_last_week | \
    sed -r -e 's/: *[-+eE.0-9]+/:10.0/g' >&5 || RES=$?

curlfail_push_expect_400
api_get_json /metric/computed/datacenter_indicators?arg1=4000\&arg2=power,blablabla >&5 || RES=$?
api_get_json /metric/computed/datacenter_indicators?arg1=4000,10\&arg2=power >&5 || RES=$?
curlfail_pop

print_result $RES
