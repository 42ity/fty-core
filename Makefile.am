AM_CFLAGS=-std=c11 -Wall -Werror -O2 -fstack-protector -D_FORTIFY_SOURCE=2
AM_CXXFLAGS=-std=c++11 -Wall -Werror -O2 -fstack-protector -D_FORTIFY_SOURCE=2

bin_PROGRAMS = netmon
netmon_LDADD = -lnetlink -ljsoncpp -lzmq
netmon_SOURCES = src/netmon/netmon.c src/netmon/jsonf.cc
netmon_CFLAGS = -Isrc/netmon

SUBDIRS = docs/man
