test_it
RES=0
for i in 5 6 7 8; do
    api_get_json /asset/device/$i >&5 || RES=$?
done
print_result $RES
