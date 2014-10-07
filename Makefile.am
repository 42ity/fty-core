ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests
EXTRA_DIST =

bin_PROGRAMS =
check_PROGRAMS =
check_LTLIBRARIES =
noinst_LTLIBRARIES =
check_SCRIPTS =
noinst_SCRIPTS =
BUILT_SOURCES =
CLEANFILES =
TESTS =

AM_CPPFLAGS = \
			  -include $(top_builddir)/config.h \
			  -I $(top_srcdir)/src/utils \
			  -I $(top_srcdir)/src/utils/messages \
        -DDEBUG \
        -DDEVEL


AM_CFLAGS = ${my_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections \
	$(LIBZMQ_CFLAGS) \
	$(LIBVARIANT_CFLAGS)

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

noinst_LTLIBRARIES += \
					  libpriv-subprocess.la

libpriv_subprocess_la_SOURCES = \
								src/utils/subprocess.h \
								src/utils/subprocess.cc
libpriv_subprocess_la_LIBADD = -lcxxtools

noinst_LTLIBRARIES += \
					  libpriv-cidr.la
libpriv_cidr_la_SOURCES = \
						 src/utils/cidr.h \
						 src/utils/cidr.cc

noinst_LTLIBRARIES += \
					  libpriv-db.la

libpriv_db_la_SOURCES = \
								 src/utils/db.h \
								 src/utils/persistence/client.cc \
								 src/utils/persistence/client.h \
								 src/utils/persistence/clientinfo.cc \
								 src/utils/persistence/clientinfo.h \
								 src/utils/persistence/databaseobject.cc \
								 src/utils/persistence/databaseobject.h \
								 src/utils/persistence/databasetimeobject.cc \
								 src/utils/persistence/databasetimeobject.h \
								 src/utils/persistence/device_discovered.cc \
								 src/utils/persistence/device_discovered.h \
								 src/utils/persistence/devicetype.cc \
								 src/utils/persistence/devicetype.h \
								 src/utils/persistence/ip.cc \
								 src/utils/persistence/ip.h \
								 src/utils/persistence/nethistory.cc \
								 src/utils/persistence/nethistory.h

BUILT_SOURCES	+=	src/utils/messages/json_schemas.h

CLEANFILES	+=	src/utils/messages/json_schemas.h

#----------------------------------------------------------------------

if HAVE_DOXYGEN
check_SCRIPTS += tools/run-test-doc.sh
#else !HAVE_DOXYGEN
# AQU note: this would need a target to work...
#@echo "Source code documentation check not available since 'doxygen' was not found."
endif

# Needed at least for the distcheck (post distribution tests)
EXTRA_DIST += tools/run-test.sh
LOG_COMPILER = $(top_srcdir)/tools/run-test.sh

check_LTLIBRARIES += \
					  libpriv-test-run.la

libpriv_test_run_la_SOURCES = \
							  tests/include/catch.hpp \
							  tests/test-run.cc
libpriv_test_run_la_CPPFLAGS =$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

#----------------------------------------------------------------------

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

check_PROGRAMS += \
		 test-log
test_log_SOURCES = \
					src/utils/log.h \
					src/utils/utils.h \
					tests/utils/test-log.cc
test_log_LDADD = \
				   libpriv-log.la \
				   libpriv-utils.la \
				   libpriv-test-run.la
test_log_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

check_PROGRAMS += \
		 test-subprocess

test_subprocess_SOURCES = \
					src/utils/subprocess.h \
					tests/utils/test-subprocess.cc
test_subprocess_LDADD = \
				   libpriv-subprocess.la \
				   libpriv-test-run.la
test_subprocess_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/
test_subprocess_LDFLAGS = -lcxxtools -lcidr

check_PROGRAMS += \
		 test-cidr
test_cidr_SOURCES = \
					src/utils/cidr.cc \
					tests/utils/test-cidr.cc
test_cidr_LDADD = \
				   libpriv-test-run.la
test_cidr_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/
test_cidr_LDFLAGS = -lcidr

check_PROGRAMS += \
		test-json-schemas

test_json_schemas_SOURCES = \
						tests/utils/test-json-schemas.cc \
						tests/utils/messages/json_schemas.h
test_json_schemas_LDADD = \
                    		libpriv-test-run.la
test_json_schemas_LDFLAGS = \
                    		$(LIBVARIANT_LIBS)
test_json_schemas_CPPFLAGS = -I $(top_srcdir)/tests/include -I $(top_srcdir)/tests/utils/messages -D TEST_JSON_SCHEMAS_BINARY_PATH=\"$(top_srcdir)\"

check_PROGRAMS += \
				  test-db
test_db_SOURCES = \
				  tests/utils/test-clientinfo.cc \
				  tests/utils/test-device-discovered.cc \
				  tests/utils/test-devicetype.cc \
			          tests/utils/test-client.cc

test_db_LDADD = \
				libpriv-db.la \
				libpriv-cidr.la \
                libpriv-test-run.la
test_db_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/ -I $(top_srcdir)/src/utils/persistence -I$(top_srcdir)/tools
test_db_LDFLAGS = -ltntdb -lcxxtools -lcidr

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)
#----------------------------------------------------------------------

bin_PROGRAMS += \
				netmon
netmon_LDFLAGS = -lnetlink $(LIBZMQ_LIBS) $(LIBVARIANT_LIBS)
netmon_SOURCES = \
				 src/netmon/linux \
				 src/netmon/netmon.c \
				 src/netmon/jsonf.h \
				 src/netmon/jsonf.cc
nodist_netmon_SOURCES = \
				src/utils/messages/json_schemas.h
netmon_CPPFLAGS = $(AM_CPPFLAGS) -I $(top_srcdir)/src/netmon -D_GNU_SOURCE
netmon_LDADD = libpriv-utils.la libpriv-log.la

SUBDIRS = docs/man

bin_PROGRAMS += \
			  cli
cli_SOURCES = \
			src/cli/cli.h \
			src/cli/cli.c \
			src/cli/cli-network.cc \
			src/utils/messages/json_schemas.h
nodist_cli_SOURCES = \
				src/utils/messages/json_schemas.h
cli_LDADD = libpriv-utils.la libpriv-cli.la libpriv-log.la
cli_LDFLAGS = $(LIBZMQ_LIBS) $(LIBVARIANT_LIBS) 

bin_PROGRAMS += \
				cored
cored_SOURCES = \
				src/cored/main.h \
				src/cored/main.cc
nodist_cored_SOURCES = \
				src/utils/messages/json_schemas.h
cored_LDADD = libpriv-cored.la
cored_LDFLAGS = -pthread $(LIBZMQ_LIBS) $(LIBVARIANT_LIBS)

bin_PROGRAMS += \
				driver-nmap
driver_nmap_SOURCES = \
					  src/drivers/nmap/nmap.cc \
					  src/drivers/nmap/nmap-parse.cc
driver_nmap_LDADD = libpriv-subprocess.la
driver_nmap_LDFLAGS = -lczmq -lcxxtools

#----------------------------------------------------------------------
EXTRA_DIST += $(top_srcdir)/tools/gen-json-validator
EXTRA_DIST += $(top_srcdir)/src/utils/messages/json_schema/*.json
BUILT_SOURCES += $(top_srcdir)/src/utils/messages/json_schemas.h

src/utils/messages/json_schemas.h: Makefile
	$(top_srcdir)/tools/gen-json-validator $(top_srcdir)/src

BUILT_SOURCES += $(top_srcdir)/tests/utils/messages/json_schemas.h
tests/utils/messages/json_schemas.h: Makefile
	$(top_srcdir)/tools/gen-json-validator $(top_srcdir)/tests

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
