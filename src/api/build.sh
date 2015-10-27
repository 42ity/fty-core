#!/bin/bash

gcc -std=c11 -D_GNU_SOURCE -ggdb -lmlm -lczmq -lzmq proto.c -o proto
