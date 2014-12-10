#!/bin/sh
test_it
api_get_json /metric/rack_total?arg1=21\&arg2=total_power | sed -r -e 's/"total_power":[.0-9]+/"total_power":10.0/' >&5
api_get_json /metric/rack_total?arg1=424242\&arg2=total_power >&5
