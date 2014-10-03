#!/bin/bash
echo "${@:1:$#-1}"
echo "${@:$#:$#}"
declare -r HARD_ERROR=99
if [[ -z "${1}" ]]; then
    echo "FATAL: name of test is missing, exiting!"
    exit ${HARD_ERROR}
fi

mkdir -p tests/junit/

exec "${1}" -r junit -o "tests/junit/${1}.xml"
