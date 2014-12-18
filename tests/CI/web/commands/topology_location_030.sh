#!/bin/sh
test_it
api_get_json /topology/location?to=7002 >&5
