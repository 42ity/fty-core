ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests
EXTRA_DIST =

bin_PROGRAMS =
check_PROGRAMS =
check_LTLIBRARIES =
noinst_LTLIBRARIES =
check_SCRIPTS =
noinst_SCRIPTS =
TESTS =

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
						 src/cli/cli.h \
						 src/cli/cli-shared.c

libpriv_cli_la_LIBADD = libpriv-utils.la

noinst_LTLIBRARIES += \
  libpriv-cored.la

libpriv_cored_la_SOURCES = \
  src/utils/zmq.hpp \
  src/cored/mock_db.h \
  src/cored/mock_db.cc \
  src/cored/utilities.h \
  src/cored/utilities.cc \
  src/cored/utilities_zeromq.h \
  src/cored/utilities_zeromq.cc

noinst_LTLIBRARIES += \
					  libpriv-log.la

libpriv_log_la_SOURCES = \
						 src/utils/log.h \
						 src/utils/log.c
libpriv_log_la_CPPFLAGS = $(AM_CPPFLAGS) -D_GNU_SOURCE

#----------------------------------------------------------------------

check_SCRIPTS += tools/run-test-doc.sh
# I am not sure whether we need this. TODO MVY Please look at this. I think that run-test.sh doesn;t belong to distribution package.
# EXTRA_DIST += tools/run-test.sh
LOG_COMPILER = $(top_srcdir)/tools/run-test.sh

check_LTLIBRARIES += \
					  libpriv-test-run.la

libpriv_test_run_la_SOURCES = \
							  tests/include/catch.hpp \
							  tests/test-run.cc
libpriv_test_run_la_CPPFLAGS =$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

check_PROGRAMS += \
		 test-utils
test_utils_SOURCES = \
					src/utils/utils.h \
					tests/utils/test-utils.cc
test_utils_LDADD = \
				   libpriv-utils.la \
				   libpriv-test-run.la
test_utils_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

check_PROGRAMS += \
		 test-cli
test_cli_SOURCES = \
				   src/cli/cli.h \
				   tests/cli/test-cli.cc
test_cli_LDADD = \
				 libpriv-utils.la \
				 libpriv-cli.la \
				 libpriv-test-run.la
test_cli_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include -I $(top_srcdir)/src/cli

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)
#----------------------------------------------------------------------
bin_PROGRAMS += \
				netmon
netmon_LDFLAGS = -lnetlink -ljsoncpp $(LIBZMQ_LIBS)
netmon_SOURCES = \
				 src/netmon/linux \
				 src/netmon/netmon.c \
				 src/netmon/jsonf.h \
				 src/netmon/jsonf.cc
netmon_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/netmon
netmon_LDADD = libpriv-utils.la libpriv-log.la

SUBDIRS = docs/man

cli_LDFLAGS = -ljsoncpp -lzmq
bin_PROGRAMS += \
			  cli
cli_SOURCES = \
			  src/cli/cli.c \
			  src/cli/cli-network.cc
cli_LDADD = libpriv-utils.la libpriv-cli.la

bin_PROGRAMS += \
				cored
cored_SOURCES = \
				src/cored/main.h \
				src/cored/main.cc
cored_LDFLAGS = -lzmq -ljsoncpp -pthread
cored_LDADD = libpriv-cored.la

#----------------------------------------------------------------------

if HAVE_DOXYGEN
EXTRA_DIST += $(top_srcdir)/${myDOXDIR}/html

html check: clean-html
	$(DOXYGEN) $(top_srcdir)/${myDOXDIR}/Doxyfile

clean-html:
	rm -rf  $(top_srcdir)/${myDOXDIR}/html

.PHONY: clean-html
 
endif

clean-local:
	rm -rf $(top_builddir)/tests/junit

if HAVE_ASTYLE
EXTRA_DIST += $(top_srcdir)/tools/astyle.conf \
	$(top_srcdir)/tools/astyle-bios
checkstyle:
	$(top_srcdir)/tools/astyle-bios -c $(top_srcdir)/tools/astyle.conf $(top_srcdir)/src
style:
	$(top_srcdir)/tools/astyle-bios $(top_srcdir)/tools/astyle.conf $(top_srcdir)/src	
endif
