#!/bin/bash
NEWARGS="${@:1:$#-1}"
NEWPROGRAM="${@:$#:$#}"
declare -r HARD_ERROR=99
if [[ -z "${NEWPROGRAM}" ]]; then
    echo "FATAL: name of test is missing, exiting!"
    exit ${HARD_ERROR}
fi

mkdir -p tests/junit/

exec "${NEWPROGRAM}" "${NEWARGS}" -r junit -o "tests/junit/${1}.xml"
