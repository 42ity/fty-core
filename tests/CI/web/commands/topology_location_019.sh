#!/bin/sh
test_it
api_get_json /topology/location?from=7029\&recursive=yes\&filter=devices >&5
