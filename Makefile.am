ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests

noinst_LTLIBRARIES =
TESTS =
manual_tests =

AM_CPPFLAGS = \
			  -include $(top_builddir)/config.h \
			  -I $(top_srcdir)/src/utils

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

noinst_LTLIBRARIES += \
  libpriv-cored.la

libpriv_cored_la_SOURCES = \
  src/cored/mock_db.h \
  src/cored/mock_db.cc \
  src/cored/mock_netmon.h \
  src/cored/mock_netmon.cc \
  src/cored/utilities.h \
  src/cored/utilities.cc \
  src/cored/utilities_zeromq.h \
  src/cored/utilities_zeromq.cc

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

bin_PROGRAMS = netmon cli cored
netmon_LDFLAGS = -lnetlink -ljsoncpp -lzmq
netmon_SOURCES = src/netmon/netmon.c src/netmon/jsonf.cc
netmon_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/netmon

SUBDIRS = docs/man

cli_LDFLAGS = -ljsoncpp
cli_LDADD = libpriv-utils.la libpriv-cli.la
cli_SOURCES = src/cli/cli.c src/cli/cli-network.cc

cored_LDFLAGS = -lzmq -ljsoncpp -pthread
cored_LDADD = libpriv-cored.la
cored_SOURCES = src/cored/main.cc
