#!/bin/sh
test_it
api_get_json /topology/power?filter_dc=5078 >&5
