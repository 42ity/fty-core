#!/bin/sh
test_it
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=power | sed -r -e 's/:[0-9]+\.?[0-9]*/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=10\&arg2=avg_power_last_day,avg_power_last_week | sed -r -e 's/:[0-9]+\.?[0-9]*/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=10,19\&arg2=avg_humidity_last_day,avg_humidity_last_week | sed -r -e 's/:[0-9]+\.?[0-9]*/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=4000\&arg2=power,blablabla >&5
api_get_json /metric/computed/datacenter_indicators?arg1=4000,10\&arg2=power >&5
