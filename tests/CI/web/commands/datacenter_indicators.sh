#!/bin/sh
test_it
api_get_json /metric/computed/datacenter_indicators?arg1=7032\&arg2=power | sed -r -e 's/:[0-9]+\.[0-9]+/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=7032\&arg2=avg_power_last_day,avg_power_last_week | sed -r -e 's/:[0-9]+\.[0-9]+/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=7032,7000\&arg2=avg_humidity_last_day,avg_humidity_last_week | sed -r -e 's/:[0-9]+\.[0-9]+/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=7032\&arg2=power,blablabla | sed -r -e 's/:[0-9]+\.[0-9]+/:10.0/g' >&5
api_get_json /metric/computed/datacenter_indicators?arg1=0,7000\&arg2=power | sed -r -e 's/:[0-9]+\.[0-9]+/:10.0/g' >&5
