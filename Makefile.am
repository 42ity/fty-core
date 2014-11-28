ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS = color-tests parallel-tests
EXTRA_DIST =

bin_PROGRAMS =
check_PROGRAMS =
check_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
check_SCRIPTS =
noinst_SCRIPTS =
BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
TESTS =

SUBDIRS = docs/man docs/examples tools

AM_CPPFLAGS = -include $(top_builddir)/config.h \
			  -I$(top_srcdir)/src/shared \
			  -I$(top_srcdir)/src/include \
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

# ----------------------------------------------------------------------
# List of header files. The purpose of this list is not dependency
# tracking (which is automatic), but to ensure these files are
# distributed by "make dist" although not "installed".

dist_noinst_HEADERS = 

#----------------------------------------------------------------------
noinst_LTLIBRARIES += 		libpriv-utils.la

libpriv_utils_la_SOURCES = 	src/shared/utils.c \
				src/shared/utils.h


noinst_LTLIBRARIES += 		libpriv-log.la

libpriv_log_la_SOURCES = 	src/shared/log.h \
				src/shared/log.c

libpriv_log_la_CPPFLAGS = $(AM_CPPFLAGS) -D_GNU_SOURCE

noinst_LTLIBRARIES += \
				libpriv-subprocess.la

libpriv_subprocess_la_SOURCES = src/shared/subprocess.h \
				src/shared/subprocess.cc

libpriv_subprocess_la_LIBADD = ${CXXTOOLS_LIBS}

noinst_LTLIBRARIES += 		libpriv-cidr.la

libpriv_cidr_la_SOURCES = 	src/shared/cidr.h \
				src/shared/cidr.cc

noinst_LTLIBRARIES += 		libpriv-db.la

libpriv_db_la_SOURCES = 	src/persist/persistence.h \
				src/persist/databaseobject.cc \
				src/persist/databaseobject.h \
				src/persist/databasetimeobject.cc \
				src/persist/databasetimeobject.h \
				src/persist/ip.cc \
				src/persist/ip.h \
				src/persist/nethistory.cc \
				src/persist/nethistory.h \
				src/persist/assetmsg.cc \
				src/persist/assetmsg.h \
				src/persist/persistencelogic.cc \
				src/persist/persistencelogic.h \
				src/persist/monitor.cc \
				src/persist/monitor.h \
				src/persist/measure_types.cc \
				src/persist/measure_types.h

libpriv_db_la_CPPFLAGS = $(AM_CPPFLAGS) \
					-I$(top_srcdir)/src/include \
					-I$(top_srcdir)/src/persist \
					-I$(top_srcdir)/src/shared \
					-I$(top_srcdir)/src/simple \
					-I$(top_srcdir)/tools \
					-I$(top_builddir)/tools

noinst_LTLIBRARIES += 		libpriv-proto.la
libpriv_proto_la_SOURCES = src/include/asset_msg.c \
						   src/include/asset_msg.h \
						   src/include/common_msg.c \
						   src/include/common_msg.h \
						   src/include/defs.h \
						   src/include/netdisc_msg.c \
						   src/include/netdisc_msg.h \
						   src/include/nmap_msg.c \
						   src/include/nmap_msg.h \
						   src/include/powerdev_msg.c \
						   src/include/powerdev_msg.h

noinst_LTLIBRARIES += 		libpriv-nmap-parse.la
libpriv_nmap_parse_la_SOURCES = src/drivers/nmap/nmap-parse.h \
					  src/drivers/nmap/nmap-parse.cc

DISTCLEANFILES +=		docs/doxygen/Doxyfile

#----------------------------------------------------------------------

if HAVE_DOXYGEN
@echo "WARNING: Source code documentation was built with 'doxygen', but the check with 'tools/run-test-doc.sh' is skipped since it is known that it currently fails!" >&2
#check_SCRIPTS += tools/run-test-doc.sh
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


AM_LOG_FLAGS = 				exclude: [db*]

check_LTLIBRARIES += 			libpriv-test-run.la

libpriv_test_run_la_SOURCES = 		tests/include/catch.hpp \
					tests/test-run.cc

libpriv_test_run_la_CPPFLAGS =		$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/

#----------------------------------------------------------------------
#                        TNTnet
#----------------------------------------------------------------------

SUFFIXES=.ecpp .gif .jpg .css .js .cpp

.ecpp.cpp:
	${ECPPC} ${ECPPFLAGS} ${ECPPFLAGS_CPP} -o $@ $<
.gif.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/gif ${ECPPFLAGS_GIF} -b -o $@ $<
.jpg.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/jpg ${ECPPFLAGS_JPG} -b -o $@ $<
.png.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/png ${ECPPFLAGS_PNG} -b -o $@ $<
.ico.cpp:
	${ECPPC} ${ECPPFLAGS} -m image/x-icon ${ECPPFLAGS_ICO} -b -o $@ $<
.css.cpp:
	${ECPPC} ${ECPPFLAGS} -m text/css ${ECPPFLAGS_CSS} -b -o $@ $<
.js.cpp:
	${ECPPC} ${ECPPFLAGS} -m application/javascript ${ECPPFLAGS_JS} -b -o $@ $<

#----------------------------------------------------------------------
#                        Unit tests
#----------------------------------------------------------------------

check_PROGRAMS += 			test-utils

test_utils_SOURCES = 	src/shared/utils.h \
						tests/shared/test-utils.cc

test_utils_LDADD = 		libpriv-utils.la \
						libpriv-test-run.la

test_utils_CPPFLAGS =	$(AM_CPPFLAGS) \
						-I$(top_srcdir)/tests/include/

check_PROGRAMS += 			test-log

test_log_SOURCES = 		src/shared/log.h \
						src/shared/utils.h \
						tests/shared/test-log.cc

test_log_LDADD = 			libpriv-log.la \
					libpriv-utils.la \
					libpriv-test-run.la
test_log_CPPFLAGS =			$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/

# EXPL: THA, ACE reported that they had observed unstable
# 		test-subprocess execution. On ACE's computer it had
# 		supposedly crashed only once; THA did not observe
# 		a test-failure on his local computer at all.
#
# 		However on Jenkins CI (running on a weaker virtual
# 		machine) this test fails quite often, but still
# 		unregularly.
#
# SOLUTION: Until we find out what's happening,
# 			don't include this test into make check
#check_PROGRAMS += 			test-subprocess
test_subprocess_SOURCES = 		src/shared/subprocess.h \
					tests/shared/test-subprocess.cc
test_subprocess_LDADD = 		libpriv-subprocess.la \
					libpriv-test-run.la
test_subprocess_CPPFLAGS =		$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/
test_subprocess_LDFLAGS =		${CXXTOOLS_LIBS} -lcidr

check_PROGRAMS += 			test-cidr
test_cidr_SOURCES = 			src/shared/cidr.cc \
					tests/shared/test-cidr.cc
test_cidr_LDADD = 			libpriv-test-run.la
test_cidr_CPPFLAGS = 			$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/
test_cidr_LDFLAGS =			-lcidr


check_PROGRAMS += 			test-db

test_db_SOURCES = 	tests/persist/test-nethistory.cc

test_db_LDADD = 			libpriv-db.la \
					libpriv-cidr.la \
					libpriv-test-run.la

test_db_CPPFLAGS = $(AM_CPPFLAGS) \
				   -I$(top_srcdir)/tests/include/ \
				   -I$(top_srcdir)/src/persist \
				   -I$(top_srcdir)/tools \
				   -I$(top_builddir)/tools

test_db_LDFLAGS =			-ltntdb ${CXXTOOLS_LIBS} -lcidr

check_PROGRAMS += 			test-db2

test_db2_SOURCES = tests/persist/test-monitor.cc \
				   tests/persist/test-monitor-clientinfo.cc \
				   tests/persist/test-realdatamonitor.cc

test_db2_LDADD = libpriv-db.la \
				 libpriv-cidr.la \
				 libpriv-proto.la \
				 libpriv-test-run.la \
				 libpriv-log.la


test_db2_CPPFLAGS =	$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/ \
					-I$(top_srcdir)/src/persist \
					-I$(top_srcdir)/src/simple \
					-I$(top_srcdir)/tools \
					-I$(top_builddir)/tools

test_db2_LDFLAGS =			-ltntdb ${CXXTOOLS_LIBS} -lcidr ${LIBCZMQ_LIBS}

###
check_PROGRAMS += 			test-database

test_database_SOURCES = 	tests/persist/test-database.cc \
							src/persist/persistencelogic.cc

test_database_LDADD = 			libpriv-db.la \
					libpriv-proto.la \
					libpriv-cidr.la \
					libpriv-test-run.la \
					libpriv-log.la

test_database_CPPFLAGS = $(AM_CPPFLAGS) \
				   -I$(top_srcdir)/tests/include/ \
				   -I$(top_srcdir)/src/persist \
				   -I$(top_srcdir)/src/include \
				   -I$(top_srcdir)/tools \
				   -I$(top_builddir)/tools

test_database_LDFLAGS =			-ltntdb ${CXXTOOLS_LIBS} -lcidr $(LIBCZMQ_LIBS)

###
check_PROGRAMS += 			test-nmap-parse
test_nmap_parse_SOURCES = tests/drivers/nmap/test-nmap-parse.cc
test_nmap_parse_LDADD = libpriv-nmap-parse.la \
						libpriv-log.la \
						libpriv-proto.la \
						libpriv-test-run.la
test_nmap_parse_LDFLAGS = $(LIBCZMQ_LIBS) $(CXXTOOLS_LIBS)
test_nmap_parse_CPPFLAGS = $(AM_CPPFLAGS) \
				   -I$(top_srcdir)/tests/include/ \
				   -I$(top_srcdir)/src/drivers/nmap/ \
				   -DTDIR=$(top_srcdir)/tests/drivers/nmap/
EXTRA_DIST += $(top_srcdir)/tests/drivers/nmap/mbt-lab.xml

###

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)

#----------------------------------------------------------------------
#                        binaries
#----------------------------------------------------------------------

bin_PROGRAMS += 			netmon
netmon_SOURCES = src/drivers/netmon/linux \
				 src/drivers/netmon/netmon.c \
				 src/drivers/netmon/msg_send.cc \
				 src/drivers/netmon/msg_send.h

netmon_CPPFLAGS = $(AM_CPPFLAGS) \
				  -I$(top_srcdir)/src/drivers/netmon \
				  -D_GNU_SOURCE \
				  -I$(top_srcdir)/src/simple

netmon_LDADD = libpriv-log.la \
			   libpriv-proto.la \
			   libpriv-utils.la

netmon_LDFLAGS =			-lnetlink $(LIBCZMQ_LIBS)

bin_PROGRAMS += simple

simple_SOURCES = src/simple/main.cc \
				 src/include/defs.h \
				 $(top_srcdir)/tools/dbinit.h \
				 src/persist/persistence.h \
				 src/drivers/nut/nut-actor.cc \
				 src/drivers/nut/nut-driver.cc
				 src/drivers/nut/nut-driver.cc \
				 src/shared/upsstatus.cc

simple_LDFLAGS = -lcidr \
				 ${LIBCZMQ_LIBS} \
				 ${LIBNUTCLIENT_LIBS} \
				 ${CXXTOOLS_LIBS} \
				 -ltntdb

simple_CPPFLAGS = $(AM_CPPFLAGS) \
				  -I$(top_srcdir)/tools/ \
				  -I$(top_builddir)/tools/ \
				  -I$(top_srcdir)/src/persist \
				  -I$(top_srcdir)/src/simple/ \
				  -I$(top_srcdir)/src/drivers/nut

simple_LDADD = libpriv-db.la \
			   libpriv-cidr.la \
			   libpriv-log.la \
			   libpriv-proto.la \
			   libpriv-subprocess.la

dist_noinst_HEADERS += src/include/asset_msg.h \
					   src/simple/asset_types.h \
					   src/include/powerdev_msg.h \
					   src/drivers/nut/nut-actor.h \
					   src/drivers/nut/nut-driver.h


bin_PROGRAMS += driver-nmap

driver_nmap_SOURCES = src/drivers/nmap/nmap-driver.h \
					  src/drivers/nmap/nmap.cc

driver_nmap_LDADD = libpriv-cidr.la \
					libpriv-log.la \
					libpriv-proto.la \
					libpriv-nmap-parse.la \
					libpriv-subprocess.la
driver_nmap_LDFLAGS = ${LIBCZMQ_LIBS} ${CXXTOOLS_LIBS} -lcidr
driver_nmap_CPPFLAGS = $(AM_CPPFLAGS) \
			   		   -I$(top_srcdir)/src/simple

# Generated header needed for any programs that use the database
BUILT_SOURCES +=	$(top_builddir)/tools/dbpath.h

pkglib_LTLIBRARIES += bios_web.la
bios_web_la_SOURCES = src/web/src/auth.cpp  src/web/src/auth-verify.cpp  \
                      src/shared/data.cpp   src/web/src/item.cpp         \
                      src/web/src/json.cpp  src/web/src/list.cpp         \
                      src/web/src/mock.cpp  src/web/src/sasl.cpp         \
                      src/web/src/network.cpp src/web/src/networks.cpp   \
                      src/web/src/time.cpp  src/web/src/tokens.cpp       \
                      src/shared/log.c      src/web/src/current.cpp

bios_web_la_CPPFLAGS = $(AM_CPPFLAGS) \
                       ${LIBCZMQ_CFLAGS} ${CXXTOOLS_CFLAGS} \
                       ${LIBSODIUM_CFLAGS} ${LIBSASL2_CFLAGS} \
					   -DSASLAUTHD_MUX=${SASLAUTHD_MUX} \
					   -I$(top_srcdir)/src/simple \
					   -I$(top_srcdir)/src/shared \
				   	   -I$(top_srcdir)/src/web/include \
					   -I$(top_srcdir)/tools \
				   	   -I$(top_builddir)/tools \
					   -I$(top_srcdir)/src/persist

bios_web_la_LDFLAGS = ${LIBCZMQ_LIBS}	${CXXTOOLS_LIBS} \
					  ${LIBSODIUM_LIBS} ${LIBSASL2_LIBS} \
					  -module -lm -ltntdb -ltntnet -lcidr
bios_web_la_LIBADD = libpriv-db.la libpriv-cidr.la

dist_noinst_HEADERS +=	src/shared/data.h \
			src/web/include/sasl.h \
			src/web/include/tokens.h \
			src/persist/assetmsg.h \
			tools/dbpath.h

tntnet.xml: $(top_srcdir)/src/web/tntnet.xml
	${SED} 's|\(.*\)<!--.*<dir>/</dir>.*-->.*|\1<dir>$(abs_top_srcdir)/src/web</dir>\n\1<compPath><entry>$(abs_top_builddir)/.libs</entry></compPath>|' $< > $@ || rm -f $@

tntnet.xml.example: $(top_srcdir)/src/web/tntnet.xml
	${SED} -e 's|\(.*\)<!--.*<dir>/</dir>.*-->.*|\1<dir>$(datarootdir)/@PACKAGE@-@VERSION@/web</dir>\n\1<compPath><entry>$(pkglibdir)</entry></compPath>|' -e 's|<!--\ <daemon>0</daemon>\ -->|<daemon>1</daemon>|' $< > $@ || rm -f $@

exampleconfdir = $(datarootdir)/@PACKAGE@-@VERSION@/examples
exampleconf_DATA = tntnet.xml.example
EXTRA_DIST += $(exampleconf_DATA)

web-test: tntnet.xml bios_web.la
	${TNTNET} tntnet.xml

#----------------------------------------------------------------------
#                        Extra files and sources
#----------------------------------------------------------------------

EXTRA_DIST +=				$(top_srcdir)/tools/gen-json-validator \
					$(top_srcdir)/tools/dbpath.h.in \
					$(top_builddir)/tools/dbpath.h \
					$(top_srcdir)/src/*_msg.xml \
					$(top_srcdir)/src/simple/*.h \
					$(top_srcdir)/src/include/*.h \
					$(top_srcdir)/src/drivers/netmon/*.h \
					$(top_srcdir)/src/drivers/netmon/*/*.h \
					$(top_srcdir)/src/drivers/netmon/*/*/*.h \
					$(top_srcdir)/src/drivers/*/*.h \
					$(top_srcdir)/src/cli/*.h \
					$(top_srcdir)/src/shared/*.h \
					$(top_srcdir)/src/web/include/*.h

# add CI tests to dist
EXTRA_DIST +=				$(top_srcdir)/tests/CI

DISTCLEANFILES +=			$(top_builddir)/tools/dbpath.h

if HAVE_DOXYGEN
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/html
endif
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/Doxyfile.in
EXTRA_DIST +=				$(top_srcdir)/docs/man/*.txt
EXTRA_DIST +=			$(top_srcdir)/INSTALL-referenceOS-debian8 \
					$(top_srcdir)/CONTRIBUTING \
					$(top_srcdir)/INSTALL

BUILT_SOURCES +=			$(top_srcdir)/tools/dbinit.h


#----------------------------------------------------------------------
#                        doxygen html generation
#----------------------------------------------------------------------

all-docs: ${myDOXDIR}/html INSTALL.html CONTRIBUTING.html \
		INSTALL-referenceOS-debian8.html man-docs

man-docs:
	( cd docs/man && $(MAKE) all )

CLEANFILES +=		$(top_srcdir)/${myDOXDIR}/doxygen_sqlite3.db \
			$(top_srcdir)/${myDOXDIR}/${mydoxylog} \
			${myDOXDIR}/doxygen_sqlite3.db \
			${myDOXDIR}/${mydoxylog}

${myDOXDIR}/html:
if HAVE_DOXYGEN
	@ if [ -f ${myDOXDIR}/Doxyfile ]; then \
		test -d ${myDOXDIR}/html || \
		$(DOXYGEN) ${myDOXDIR}/Doxyfile; \
	else \
		if [ -f $(top_srcdir)/${myDOXDIR}/Doxyfile ]; then \
			test -d $(top_srcdir)/${myDOXDIR}/html || \
			$(DOXYGEN) $(top_srcdir)/${myDOXDIR}/Doxyfile; \
		else \
			echo "FAIL: No '${myDOXDIR}/Doxyfile' nor '$(top_srcdir)/${myDOXDIR}/Doxyfile' found" >&2; \
			/bin/false; \
		fi; \
	fi
else
	@echo "SKIP: Missing 'doxygen' program, sourcecode-documentation generation skipped!" >&2
endif

if HAVE_ASCIIDOC
INSTALL.html: INSTALL
	$(ASCIIDOC) -b html -o $@ $^

CONTRIBUTING.html: CONTRIBUTING
	$(ASCIIDOC) -b html -o $@ $^

INSTALL-referenceOS-debian8.html: INSTALL-referenceOS-debian8
	$(ASCIIDOC) -b html -o $@ $^
else
INSTALL.html:
	@echo "SKIP: Missing 'asciidoc' program, generic HTML documentation generation skipped!" >&2

CONTRIBUTING.html:
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
	/bin/rm -f INSTALL.html
	/bin/rm -f CONTRIBUTING.html
	/bin/rm -f INSTALL-referenceOS-debian8.html

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
