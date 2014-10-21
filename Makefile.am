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
DISTCLEANFILES =
TESTS =

SUBDIRS = docs/man

AM_CPPFLAGS = 			-include $(top_builddir)/config.h \
				-I $(top_srcdir)/src/utils \
				-I $(top_srcdir)/src/utils/messages \
				-DDEBUG \
				-DDEVEL


AM_CFLAGS =			${my_CFLAGS} \
				-fvisibility=hidden \
				-ffunction-sections \
				-fdata-sections \
				$(LIBZMQ_CFLAGS) \
				$(LIBCZMQ_CFLAGS) \
				$(LIBVARIANT_CFLAGS)

AM_CXXFLAGS =			${my_CXXFLAGS} \
				-fvisibility=hidden \
				-ffunction-sections \
				-fdata-sections

AM_LDFLAGS = 			-Wl,--gc-sections \
				-Wl,--as-needed


#----------------------------------------------------------------------
noinst_LTLIBRARIES += 		libpriv-utils.la

libpriv_utils_la_SOURCES = 	src/utils/utils.c \
				src/utils/utils.h


noinst_LTLIBRARIES += 		libpriv-log.la

libpriv_log_la_SOURCES = 	src/utils/log.h \
				src/utils/log.c

libpriv_log_la_CPPFLAGS = $(AM_CPPFLAGS) -D_GNU_SOURCE

noinst_LTLIBRARIES += \
				libpriv-subprocess.la

libpriv_subprocess_la_SOURCES = src/utils/subprocess.h \
				src/utils/subprocess.cc

libpriv_subprocess_la_LIBADD = -lcxxtools

noinst_LTLIBRARIES += 		libpriv-cidr.la

libpriv_cidr_la_SOURCES = 	src/utils/cidr.h \
				src/utils/cidr.cc

noinst_LTLIBRARIES += 		libpriv-db.la

libpriv_db_la_SOURCES = 	src/utils/persistence.h \
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


DISTCLEANFILES +=		docs/doxygen/Doxyfile

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


#Test case examples:
#
#thisTestOnly            Matches the test case called, 'thisTestOnly'
#"this test only"        Matches the test case called, 'this test only'
#these*                  Matches all cases starting with 'these'
#exclude:notThis         Matches all tests except, 'notThis'
#~notThis                Matches all tests except, 'notThis'
#~*private*              Matches all tests except those that contain 'private'
#a* ~ab* abc             Matches all tests that start with 'a', except those that
#                        start with 'ab', except 'abc', which is included
#
#Names within square brackets are interpreted as tags. A series of tags form an AND expression wheras a comma-separated sequence forms an OR expression. e.g.:
#
#[one][two],[three]


AM_LOG_FLAGS = 				exclude:[db*]  -t

check_LTLIBRARIES += 			libpriv-test-run.la

libpriv_test_run_la_SOURCES = 		tests/include/catch.hpp \
					tests/test-run.cc

libpriv_test_run_la_CPPFLAGS =		$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

#----------------------------------------------------------------------
#                        Unit tests
#----------------------------------------------------------------------

check_PROGRAMS += 			test-utils
test_utils_SOURCES = 			src/utils/utils.h \
					tests/utils/test-utils.cc
test_utils_LDADD = 			libpriv-utils.la \
					libpriv-test-run.la
test_utils_CPPFLAGS =			$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

check_PROGRAMS += 			test-log
test_log_SOURCES = 			src/utils/log.h \
					src/utils/utils.h \
					tests/utils/test-log.cc
test_log_LDADD = 			libpriv-log.la \
					libpriv-utils.la \
					libpriv-test-run.la
test_log_CPPFLAGS =			$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/

check_PROGRAMS += 			test-subprocess
test_subprocess_SOURCES = 		src/utils/subprocess.h \
					tests/utils/test-subprocess.cc
test_subprocess_LDADD = 		libpriv-subprocess.la \
					libpriv-test-run.la
test_subprocess_CPPFLAGS =		$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/
test_subprocess_LDFLAGS =		-lcxxtools -lcidr

check_PROGRAMS += 			test-cidr
test_cidr_SOURCES = 			src/utils/cidr.cc \
					tests/utils/test-cidr.cc
test_cidr_LDADD = 			libpriv-test-run.la
test_cidr_CPPFLAGS = 			$(AM_CPPFLAGS) -I $(top_srcdir)/tests/include/
test_cidr_LDFLAGS =			-lcidr


check_PROGRAMS += 			test-db
test_db_SOURCES = 			tests/utils/test-clientinfo.cc \
					tests/utils/test-device-discovered.cc \
					tests/utils/test-devicetype.cc \
					tests/utils/test-client.cc \
					tests/utils/test-nethistory.cc
test_db_LDADD = 			libpriv-db.la \
					libpriv-cidr.la \
					libpriv-test-run.la
test_db_CPPFLAGS =			$(AM_CPPFLAGS) \
					-I $(top_srcdir)/tests/include/ \
					-I $(top_srcdir)/src/utils/persistence \
					-I $(top_srcdir)/tools \
					-I $(top_builddir)/tools
test_db_LDFLAGS =			-ltntdb -lcxxtools -lcidr

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)

#----------------------------------------------------------------------
#                        binaries
#----------------------------------------------------------------------

bin_PROGRAMS += 			netmon
netmon_LDFLAGS =			-lnetlink $(LIBZMQ_LIBS) ${LIBCZMQ_LIBS}
netmon_SOURCES = 			src/netmon/linux \
					src/netmon/netmon.c \
					src/simple/netdisc_msg.h \
					src/simple/netdisc_msg.c \
					src/netmon/netmon_msg_send.cc \
					src/netmon/netmon_msg_send.h
# Consider moving zproto protocols to libpriv-...
netmon_CPPFLAGS =			$(AM_CPPFLAGS) -I $(top_srcdir)/src/netmon -D_GNU_SOURCE -I $(top_srcdir)/src/simple
netmon_LDADD =				libpriv-utils.la libpriv-log.la

bin_PROGRAMS += 			simple

simple_SOURCES = 			src/simple/main.cc \
					src/simple/defs.h \
					$(top_srcdir)/tools/dbinit.h \
					src/utils/persistence.h \
					src/simple/netdisc_msg.c \
					src/simple/netdisc_msg.h \
					src/utils/persistencelogic.cc \
					src/utils/persistencelogic.h \
					src/simple/powerdev_msg.c \
					src/drivers/nut/nut-actor.cc \
					src/drivers/nut/nut-driver.cc


simple_LDFLAGS =			-lcidr ${LIBCZMQ_LIBS} ${LIBNUTCLIENT_LIBS} -lcxxtools -ltntdb
#simple_LDFLAGS =			-lcidr ${LIBCZMQ_LIBS} ${LIBZMQ_LIBS} -lcxxtools -ltntdb
simple_CPPFLAGS =			-I $(top_srcdir)/tools/ -I $(top_builddir)/tools/ -I $(top_srcdir)/src/utils/ -I $(top_srcdir)/src/simple/ -I $(top_srcdir)/src/drivers/nut
simple_LDADD =				libpriv-db.la libpriv-cidr.la libpriv-log.la libpriv-subprocess.la

bin_PROGRAMS += 			driver-nmap

driver_nmap_SOURCES = 			src/drivers/nmap/nmap-driver.h \
					src/drivers/nmap/nmap.cc \
					src/drivers/nmap/nmap-parse.h \
					src/drivers/nmap/nmap-parse.cc \
					src/simple/nmap_msg.h \
					src/simple/nmap_msg.c

driver_nmap_LDADD =			libpriv-subprocess.la libpriv-log.la
driver_nmap_LDFLAGS =			${LIBCZMQ_LIBS} -lcxxtools
driver_nmap_CPPFLAGS =			-I $(top_srcdir)/src/simple -I $(top_srcdir)/src/utils


#----------------------------------------------------------------------
#                        Extra files and sources
#----------------------------------------------------------------------

EXTRA_DIST +=				$(top_srcdir)/tools/gen-json-validator \
					$(top_srcdir)/tools/dbpath.h.in \
					$(top_srcdir)/simple/*_msg.xml

CLEANFILES +=				tools/dbpath.h


if HAVE_DOXYGEN
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/html
endif
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/Doxyfile.in


BUILT_SOURCES +=			$(top_srcdir)/tools/dbinit.h


#----------------------------------------------------------------------
#                        doxygen html generation
#----------------------------------------------------------------------

all-docs: ${myDOXDIR}/html INSTALL.html INSTALL-referenceOS-debian8.html man-docs

man-docs:
	( cd docs/man && $(MAKE) all )

${myDOXDIR}/html:
if HAVE_DOXYGEN
	test -d $(top_srcdir)/${myDOXDIR}/html || $(DOXYGEN) $(top_srcdir)/${myDOXDIR}/Doxyfile
else
	@echo "SKIP: Missing 'doxygen' program, sourcecode-documentation generation skipped!" >&2
endif

if HAVE_ASCIIDOC
INSTALL.html: INSTALL
	$(ASCIIDOC) -b html $^

INSTALL-referenceOS-debian8.html: INSTALL-referenceOS-debian8
	$(ASCIIDOC) -b html $^
else
INSTALL.html:
	@echo "SKIP: Missing 'asciidoc' program, generic HTML documentation generation skipped!" >&2

INSTALL-referenceOS-debian8.html:
	@echo "SKIP: Missing 'asciidoc' program, generic HTML documentation generation skipped!" >&2
endif

#----------------------------------------------------------------------
#                        hook for make all => generate files too
#----------------------------------------------------------------------

all-local: all-docs


#----------------------------------------------------------------------
#                        clean, what was not removed automatically
#----------------------------------------------------------------------

clean-man:
	( cd docs/man && { $(MAKE) clean || /bin/true; } && /bin/rm -f *.[123456789] *.xml *.html )
	/bin/rm -f docs/man/*.?

clean-local: clean-man
	/bin/rm -rf $(top_builddir)/tests/junit
	( test -w $(top_srcdir)/${myDOXDIR}/html && /bin/rm -rf $(top_srcdir)/${myDOXDIR}/html ) || /bin/true
	/bin/rm -f INSTALL.html INSTALL-referenceOS-debian8.html

#----------------------------------------------------------------------
#                        astyle formating
#----------------------------------------------------------------------
if HAVE_ASTYLE
EXTRA_DIST += $(top_srcdir)/tools/astyle.conf \
	$(top_srcdir)/tools/astyle-bios
checkstyle:
	$(top_srcdir)/tools/astyle-bios -c $(top_srcdir)/tools/astyle.conf $(top_srcdir)/src
style:
	$(top_srcdir)/tools/astyle-bios $(top_srcdir)/tools/astyle.conf $(top_srcdir)/src
endif
