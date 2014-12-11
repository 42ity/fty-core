#!/bin/sh
test_it
api_get_json '/metric/current?dev=6' >&5
api_get_json '/metric/current?dev=7' >&5
print_result $?
api_get_json '/metric/current?dev=3' >&5
print_result $?
api_get_json '/metric/current' >&5
print_result $?
