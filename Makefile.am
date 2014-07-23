AM_CFLAGS="-Wall -Werror -O2 -fstack-protector -D_FORTIFY_SOURCE=2"

bin_PROGRAMS = netmon
netmon_LDADD = -lnetlink
netmon_SOURCES = src/netmon/netmon.c
netmon_CFLAGS = -Isrc/netmon

