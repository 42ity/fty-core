#!/bin/sh
test_it "current_epdu"
api_get_json '/metric/current?dev=6' >&5
print_result $?

test_it "current_ups"
api_get_json '/metric/current?dev=7' >&5
print_result $?

test_it "current_not_existing"
api_get_json '/metric/current?dev=3' >&5
print_result $?

test_it "current_invalid"
api_get_json '/metric/current' >&5
print_result $?
