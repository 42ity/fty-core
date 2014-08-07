ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests

bin_PROGRAMS =
noinst_LTLIBRARIES =
TESTS =
manual_tests =

AM_CPPFLAGS = \
			  -include $(top_builddir)/config.h \
			  -I $(top_srcdir)/src/utils

AM_CFLAGS = ${my_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections \
	$(LIBZMQ_CFLAGS)

AM_CXXFLAGS = ${my_CXXFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed


#----------------------------------------------------------------------
noinst_LTLIBRARIES += \
					libpriv-utils.la

libpriv_utils_la_SOURCES = \
					src/utils/utils.c \
					src/utils/utils.h

noinst_LTLIBRARIES += \
					  libpriv-cli.la

libpriv_cli_la_SOURCES = \
						 src/utils/utils.h \
						 src/utils/cli.h \
						 src/cli/cli-shared.c

libpriv_cli_la_LIBADD = libpriv-utils.la

#----------------------------------------------------------------------
TESTS += \
		 test-utils
test_utils_SOURCES = \
					src/utils/utils.h \
					tests/utils/test_utils1.c
test_utils_LDADD = libpriv-utils.la

TESTS += \
		 test-cli
test_cli_SOURCES = \
				   src/cli/cli.h \
				   tests/cli/test-cli.c
test_cli_LDADD = libpriv-cli.la
test_cli_CPPFLAGS = -I $(top_srcdir)/src/cli

noinst_PROGRAMS = $(TESTS)

#----------------------------------------------------------------------
bin_PROGRAMS += \
				netmon
netmon_LDFLAGS = -lnetlink -ljsoncpp $(LIBZMQ_LIBS)
netmon_SOURCES = src/netmon/netmon.c src/netmon/jsonf.cc
netmon_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/netmon

SUBDIRS = docs/man

bin_PROGRAMS += \
			  cli
cli_LDFLAGS = -ljsoncpp
cli_LDADD = libpriv-utils.la libpriv-cli.la
cli_SOURCES = src/cli/cli.c src/cli/cli-network.cc
