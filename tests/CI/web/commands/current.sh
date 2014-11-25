#!/bin/sh
test_it
api_get_json '/metric/current?dev=4' >&5
