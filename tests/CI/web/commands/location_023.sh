#!/bin/sh
test_it
api_get_json /topology/location?from=7025\&recursive=yes >&5
