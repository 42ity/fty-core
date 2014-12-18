#!/bin/sh
test_it
api_get_json /topology/location?from=7032\&recursive=no\&filter=rows >&5
