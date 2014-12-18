#!/bin/sh
test_it
api_get_json /topology/power?to=5062 >&5
