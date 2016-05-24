#!/bin/bash
make test-tntmlm && valgrind --leak-check=full --show-reachable=yes --error-exitcode=1 --suppressions="./.valgrind.supp" ./test-tntmlm
