test_it
RES=0
api_get_json /metric/computed/rack_total?arg1=21\&arg2=total_power | \
    sed -r -e 's/"total_power": *[-+eE.0-9]+/"total_power":10.0/' >&5 || RES=$?

curlfail_push_expect_400
api_get_json /metric/computed/rack_total?arg1=424242\&arg2=total_power >&5 || RES=$?
curlfail_pop

api_get_json /metric/computed/rack_total?arg1=21,4\&arg2=avg_power_last_week,avg_power_last_day | \
  sed -r -e 's/"avg_power_last_week": *[-+eE.0-9]+/"avg_power_last_week":10.0/g' \
         -e 's/"avg_power_last_day": *[-+eE.0-9]+/"avg_power_last_day":23.4/g'  >&5 || RES=$?
print_result $RES
