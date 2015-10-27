#!/bin/bash

gcc -ggdb -lmlm -lczmq -lzmq proto.c -o proto
