#!/bin/bash

gcc -std=c11 -D_GNU_SOURCE -ggdb -c proto.c -o proto.o
gcc -std=c11 -D_GNU_SOURCE -ggdb -c selftest.c -o selftest.o
gcc proto.o selftest.o -lmlm -lczmq -lzmq -o selftest
