This directory currently contains only simple key/value storage - basically
thread safe std::map and unit tests. Makefile compiles only unit tests (which
requires gtest), and in general is planned to be dropped and replaced with
better integrated autotools generated one.

Whole kvdb is to be dropped later, so this is here only temporally, no big
effort should be invested into this.
