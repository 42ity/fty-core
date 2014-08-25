#!/bin/bash

DOXYLOG="docs/doxygen/doxylog.txt"

declare -r ERR_S=98

if [ -e  ${DOXYLOG} ]; then
	if [ -s ${DOXYLOG} ]; then
		echo "There are undocumented entities:"
		cat ${DOXYLOG}
		exit ${ERR_S}
	fi
fi
