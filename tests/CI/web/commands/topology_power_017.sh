#!/bin/sh
test_it
api_get_json /topology/power?to=5074 >&5
