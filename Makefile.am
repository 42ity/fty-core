ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests

AM_CFLAGS=-std=c11 -Wall -Wextra -Wunused-function -O2 -fstack-protector -D_FORTIFY_SOURCE=2
AM_CXXFLAGS=-std=c++11 -Wall -Wextra -Wunused-function -O2 -fstack-protector -D_FORTIFY_SOURCE=2

noinst_LTLIBRARIES =

#----------------------------------------------------------------------
noinst_LTLIBRARIES += \
					libpriv-utils.la

libpriv_utils_la_SOURCES = \
					src/utils/utils.c \
					src/utils/utils.h
libpriv_utils_la_CFLAGS = $(AM_CFLAGS)

bin_PROGRAMS = netmon cli
netmon_LDFLAGS = -lnetlink -ljsoncpp -lzmq
netmon_SOURCES = src/netmon/netmon.c src/netmon/jsonf.cc
netmon_CFLAGS = $(AM_CFLAGS) -Isrc/netmon

cli_LDFLAGS = -ljsoncpp
cli_LDADD = libpriv-utils.la
cli_SOURCES = src/cli/cli.c src/cli/cli-network.cc
cli_CFLAGS = $(AM_CFLAGS) -Isrc/utils 
cli_CXXFLAGS = $(AM_CXXFLAGS) -Isrc/utils 
