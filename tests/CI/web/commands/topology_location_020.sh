#!/bin/sh
test_it
api_get_json /topology/location?from=7000\&recursive=no\&filter=racks >&5
