#!/bin/sh

# Simple wrapper for cmpjson.sh to do comparison of two
# JSON markups in their original ordering of items.
# Author(s): Jim Klimov <EvgenyKlimov@eaton.com>

CMPJSON="`dirname $0`/cmpjson.sh"
[ ! -x "$CMPJSON" ] && echo "ERROR: Can't find '$CMPJSON'!" >&2 && exit 1

JSONSH_OPTIONS='-N -Nnx=%.16f' \
JSONSH_OPTIONS_VERBOSE='-Nnx=%.16f' \
exec "$CMPJSON" "$@"
