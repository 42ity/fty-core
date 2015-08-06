curlfail_push_expect_400

test_it "metric_computed_average-missing__1"
api_get '/metric/computed/average?end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__2"
api_get '/metric/computed/average?start_ts=20130101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__3"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__4"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-missing__5"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26' | grep '400 Bad Request'
print_result $?


test_it "metric_computed_average-wrong__1"
api_get '/metric/computed/average?start_ts=0&end_ts=20160101000000Z&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__2"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=0&type=arithmetic_mean&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__3"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=american_average&step=8h&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

test_it "metric_computed_average-wrong__4"
api_get '/metric/computed/average?start_ts=20130101000000Z&end_ts=20160101000000Z&type=arithmetic_mean&step=5ft&element_id=26&source=temperature.thermal_zone0' | grep '400 Bad Request'
print_result $?

curlfail_pop
