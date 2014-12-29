#!/bin/sh
test_it
api_get_json /topology/power?from=5024 >&5
