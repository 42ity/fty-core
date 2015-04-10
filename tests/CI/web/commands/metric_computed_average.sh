#!/bin/sh
test_it "metric_computed_average-missing"
api_get '/metric/computed/average?end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong"

api_get '/metric/computed/average?start_ts=0&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=0&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=american_average&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=5ft&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-correct"

api_get_jsonv '/metric/computed/average?end_ts=20160101000000Z&start_ts=20130101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' >&5
