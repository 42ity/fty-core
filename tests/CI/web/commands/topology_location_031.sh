#!/bin/sh
test_it
api_get_json /topology/location?to=7025 >&5
