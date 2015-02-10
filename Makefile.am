# Description: The Makefile (automake template) for the $BIOS project.
# Note: relies on GNU make syntax and features, may fail to work in
# other make programs.

ACLOCAL_AMFLAGS =	-I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS =	color-tests parallel-tests

# http://www.gnu.org/software/automake/manual/html_node/Basics-of-Distribution.html
# NOTE: "dist" is not "install": The dist rule in the Makefile can be used to
# generate a gzipped tar file and other flavors of archive for distribution.
# Sometimes there are files that must be distributed, but which are not covered
# in the automatic rules. These files should be listed in the EXTRA_DIST
# variable. You can mention files from subdirectories in EXTRA_DIST.
# You can also mention a directory in EXTRA_DIST; in this case the entire
# directory will be recursively copied into the distribution... you can
# use the dist-hook feature to filter the contents (remove ".svn/" etc).
EXTRA_DIST =

bin_PROGRAMS =
check_PROGRAMS =
check_LTLIBRARIES =
noinst_LTLIBRARIES =
pkglib_LTLIBRARIES =
check_SCRIPTS =
noinst_SCRIPTS =
# http://www.gnu.org/software/automake/manual/html_node/Built-Sources-Example.html
# http://www.gnu.org/software/automake/manual/html_node/Sources.html
# A source file listed in BUILT_SOURCES is made on `make all` or `make check`
# (or even `make install`) before other targets are processed. However, such
# a source file is not compiled unless explicitly requested by mentioning it
# in some other _SOURCES variable. 
BUILT_SOURCES =
# http://www.gnu.org/software/automake/manual/html_node/Clean.html
# * If make built it, and it is commonly something that one would want to
#   rebuild (for instance, a .o file), then mostlyclean should delete it.
# * Otherwise, if make built it, then clean should delete it.
# * If configure built it, then distclean should delete it.
CLEANFILES =
DISTCLEANFILES =
DISTCLEANDIRS =

TESTS =

# Named by analogy from configure'd myDOXDIR for doxygen
myMANDIR =	docs/man
myDEVDOCDIR =	docs/develop

# Locations of child Makefiles
SUBDIRS =	$(myMANDIR) docs/examples tools

if ENABLE_DOCKER_SUPPORT
# Integrate packaging of resource files for management of Docker containers
# Note that docker/README is processed by this root Makefile, below...
SUBDIRS +=	docker
endif

# Clean up products of configure script
DISTCLEANFILES += \
		$(addsuffix /Makefile.in,$(SUBDIRS) .) \
		aclocal.m4 config.h.in config.h.in~ config.log \
		m4/libtool.m4 \
		m4/ltoptions.m4 \
		m4/ltsugar.m4 \
		m4/ltversion.m4 \
		m4/lt~obsolete.m4 \
		build-aux/compile* \
		build-aux/config* \
		build-aux/depcomp* \
		build-aux/install* \
		build-aux/ltmain* \
		build-aux/missing* \
		build-aux/test* \
		autom4te.cache/output* \
		autom4te.cache/traces* \
		autom4te.cache/requests \
		tests/CI/web/log/*.log

# These are deleted by "distclean-local-dirs" if they exist and are empty
# The "distclean-generic" with "rm DISTCLEANFILES" is a dependency for this
DISTCLEANDIRS += \
		autom4te.cache \
		build-aux \
		config \
		tests/CI/web/log

AM_CPPFLAGS = \
		-include $(abs_top_builddir)/config.h \
		-I$(abs_top_builddir)/src/shared \
		-I$(abs_top_builddir)/src/include \
		-I$(abs_top_srcdir)/src/shared \
		-I$(abs_top_srcdir)/src/include \
		-DDEBUG \
		-DDEVEL

AM_CFLAGS = \
		${my_CFLAGS} \
		-fvisibility=hidden \
		-ffunction-sections \
		-fdata-sections \
		$(LIBZMQ_CFLAGS) \
		$(LIBCZMQ_CFLAGS) \
		$(LIBVARIANT_CFLAGS)

AM_CXXFLAGS = \
		${my_CXXFLAGS} \
		-fvisibility=hidden \
		-ffunction-sections \
		-fdata-sections

AM_LDFLAGS = \
	 	-Wl,--gc-sections \
		-Wl,--as-needed

# ----------------------------------------------------------------------
# List of header files. The purpose of this list is not dependency
# tracking (which is automatic), but to ensure these files are
# distributed by "make dist" although not "installed".

dist_noinst_HEADERS = 

#----------------------------------------------------------------------
noinst_LTLIBRARIES += 		libpriv-defs.la

libpriv_defs_la_SOURCES = 	src/include/defs.c \
				src/include/defs.h

noinst_LTLIBRARIES += 		libpriv-git-details-override.la

libpriv_git_details_override_la_SOURCES = \
				src/include/git_details_override.h \
				src/include/git_details_override.c

noinst_LTLIBRARIES += 		libpriv-utils.la

libpriv_utils_la_SOURCES = 	src/shared/utils.c \
				src/shared/utils.h


noinst_LTLIBRARIES += 		libpriv-log.la

libpriv_log_la_SOURCES = 	src/shared/log.h \
				src/shared/log.c

noinst_LTLIBRARIES += \
				libpriv-subprocess.la

libpriv_subprocess_la_SOURCES = src/shared/subprocess.h \
				src/shared/subprocess.cc

libpriv_subprocess_la_LIBADD = ${CXXTOOLS_LIBS}

noinst_LTLIBRARIES += 		libpriv-cidr.la

libpriv_cidr_la_SOURCES = 	src/shared/cidr.h \
				src/shared/cidr.cc

noinst_LTLIBRARIES += 		libpriv-db.la

libpriv_db_la_SOURCES = \
			src/persist/persistence.h \
			src/persist/databaseobject.cc \
			src/persist/databaseobject.h \
			src/persist/databasetimeobject.cc \
			src/persist/databasetimeobject.h \
			src/persist/ip.cc \
			src/persist/ip.h \
			src/persist/nethistory.cc \
			src/persist/nethistory.h \
			src/persist/assetcrud.cc \
			src/persist/assetcrud.h \
			src/persist/assettopology.cc \
			src/persist/assettopology.h \
			src/persist/persist_error.cc \
			src/persist/persist_error.h \
			src/persist/assettopology.h \
			src/persist/persistencelogic.cc \
			src/persist/persistencelogic.h \
			src/persist/monitor.cc \
			src/persist/monitor.h \
			src/persist/calc_power.cc \
			src/persist/calc_power.h \
			src/persist/measure_types.cc \
			src/persist/measure_types.h


libpriv_db_la_CPPFLAGS = $(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/src/simple \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_builddir)/tools

noinst_LTLIBRARIES += 	libpriv-proto.la
libpriv_proto_la_SOURCES = \
			src/include/asset_msg.c \
			src/include/asset_msg.h \
			src/include/common_msg.c \
			src/include/common_msg.h \
			src/include/netdisc_msg.c \
			src/include/netdisc_msg.h \
			src/include/nmap_msg.c \
			src/include/nmap_msg.h \
			src/include/compute_msg.c \
			src/include/compute_msg.h \
			src/include/powerdev_msg.c \
			src/include/powerdev_msg.h

noinst_LTLIBRARIES +=	libpriv-nmap-parse.la
libpriv_nmap_parse_la_SOURCES = \
			src/drivers/nmap/nmap-parse.h \
			src/drivers/nmap/nmap-parse.cc

DISTCLEANFILES +=	docs/doxygen/Doxyfile

#----------------------------------------------------------------------

# Tester for the doxygen output sanity
check_SCRIPTS +=	tools/run-test-doc.sh

# Needed at least for the distcheck (post distribution tests)
EXTRA_DIST +=		tools/run-test.sh
LOG_COMPILER =		$(abs_top_srcdir)/tools/run-test.sh


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
# setup using AM_LOG_FLAGS see an example
# AM_LOG_FLAGS = 				~[db]

check_LTLIBRARIES += 			libpriv-test-run.la

libpriv_test_run_la_SOURCES = 		tests/include/catch.hpp \
					tests/test-run.cc

libpriv_test_run_la_CPPFLAGS =		$(AM_CPPFLAGS) \
					-I$(abs_top_srcdir)/tests/include/

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

check_PROGRAMS += 	test-utils

test_utils_SOURCES = 	src/shared/utils.h \
			tests/shared/test-utils.cc

test_utils_LDADD = 	libpriv-utils.la \
			libpriv-test-run.la

test_utils_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/

check_PROGRAMS += 	test-log

test_log_SOURCES = 	src/shared/log.h \
			src/shared/utils.h \
			tests/shared/test-log.cc

test_log_LDADD = 	libpriv-log.la \
			libpriv-utils.la \
			libpriv-test-run.la
test_log_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/

check_PROGRAMS += 			test-subprocess
test_subprocess_SOURCES = 		src/shared/subprocess.h \
					tests/shared/test-subprocess.cc
test_subprocess_LDADD = 		libpriv-subprocess.la \
					libpriv-test-run.la
test_subprocess_CPPFLAGS =		$(AM_CPPFLAGS) \
					-I$(abs_top_srcdir)/tests/include/
test_subprocess_LDFLAGS =		${CXXTOOLS_LIBS} -lcidr

check_PROGRAMS += 	test-cidr
test_cidr_SOURCES = 	src/shared/cidr.cc \
			tests/shared/test-cidr.cc
test_cidr_LDADD = 	libpriv-test-run.la
test_cidr_CPPFLAGS = 	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/
test_cidr_LDFLAGS =	-lcidr


bin_PROGRAMS += 	test-db

test_db_SOURCES = 	tests/persist/test-nethistory.cc

test_db_LDADD = \
			libpriv-db.la \
			libpriv-cidr.la \
			libpriv-test-run.la

test_db_CPPFLAGS = \
			$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_builddir)/tools

test_db_LDFLAGS =	-ltntdb ${CXXTOOLS_LIBS} -lcidr

bin_PROGRAMS += 	test-db2

test_db2_SOURCES = \
			tests/persist/test-monitor.cc \
			tests/persist/test-monitor-clientinfo.cc \
			tests/persist/test-realdatamonitor.cc

test_db2_LDADD = \
			libpriv-db.la \
			libpriv-cidr.la \
			libpriv-proto.la \
			libpriv-test-run.la \
			libpriv-log.la


test_db2_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/src/simple \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_builddir)/tools

test_db2_LDFLAGS =	-ltntdb ${CXXTOOLS_LIBS} -lcidr ${LIBCZMQ_LIBS}

bin_PROGRAMS += 	test-dbtopology

test_dbtopology_SOURCES = \
			tests/persist/test-topology.cc \
			tests/persist/test-topology-power-to.cc \
			tests/persist/test-topology-power-from.cc \
			tests/persist/test-topology-power-datacenter.cc \
			tests/persist/test-topology-power-group.cc \
			tests/persist/test-topology-location-from.cc \
			tests/persist/test-topology-location-to.cc


test_dbtopology_LDADD = \
			libpriv-db.la \
			libpriv-proto.la \
			libpriv-test-run.la \
			libpriv-log.la


test_dbtopology_CPPFLAGS = \
			$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/src/simple \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_builddir)/tools

test_dbtopology_LDFLAGS = \
			-ltntdb ${CXXTOOLS_LIBS} -lcidr ${LIBCZMQ_LIBS}

#### total power tests
bin_PROGRAMS += 	test-totalpower

test_totalpower_SOURCES = \
			tests/persist/test-rack-power.cc 


test_totalpower_LDADD = \
			libpriv-db.la \
			libpriv-proto.la \
			libpriv-test-run.la \
			libpriv-log.la


test_totalpower_CPPFLAGS = \
			$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/src/simple \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_builddir)/tools

test_totalpower_LDFLAGS = \
			-ltntdb ${CXXTOOLS_LIBS} -lcidr ${LIBCZMQ_LIBS}


###
bin_PROGRAMS += 	test-database

test_database_SOURCES = tests/persist/test-database.cc \
			src/persist/persistencelogic.cc

test_database_LDADD = 	libpriv-db.la \
			libpriv-proto.la \
			libpriv-cidr.la \
			libpriv-test-run.la \
			libpriv-log.la

test_database_CPPFLAGS = $(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_builddir)/tools \
			-I$(abs_top_srcdir)/tools

test_database_LDFLAGS =	-ltntdb ${CXXTOOLS_LIBS} -lcidr $(LIBCZMQ_LIBS)

###
bin_PROGRAMS +=		test-web

test_web_SOURCES = 	tests/web/src/test-location-helpers.cc \
			src/web/src/location_helpers.cpp

test_web_LDADD = 	libpriv-test-run.la \
			libpriv-log.la

test_web_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/web/include \
			-I$(abs_top_srcdir)/src/simple

#test_web_LDFLAGS =	-ltntdb ${CXXTOOLS_LIBS} -lcidr $(LIBCZMQ_LIBS)


###
check_PROGRAMS += 	test-nmap-parse
test_nmap_parse_SOURCES = tests/drivers/nmap/test-nmap-parse.cc
test_nmap_parse_LDADD = \
			libpriv-nmap-parse.la \
			libpriv-defs.la \
			libpriv-log.la \
			libpriv-proto.la \
			libpriv-test-run.la
test_nmap_parse_LDFLAGS = $(LIBCZMQ_LIBS) $(CXXTOOLS_LIBS)
test_nmap_parse_CPPFLAGS = $(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/tests/include/ \
			-I$(abs_top_srcdir)/src/drivers/nmap/ \
			-DTDIR=$(abs_top_srcdir)/tests/drivers/nmap/
EXTRA_DIST +=		tests/drivers/nmap/mbt-lab.xml

###

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)

#----------------------------------------------------------------------
#                        binaries
#----------------------------------------------------------------------

bin_PROGRAMS += 	netmon
netmon_SOURCES = \
			src/drivers/netmon/linux \
			src/drivers/netmon/netmon.c \
			src/drivers/netmon/msg_send.cc \
			src/drivers/netmon/msg_send.h

netmon_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/src/drivers/netmon \
			-I$(abs_top_srcdir)/src/simple

netmon_LDADD = \
			libpriv-defs.la \
			libpriv-log.la \
			libpriv-proto.la \
			libpriv-utils.la

netmon_LDFLAGS =	-lnetlink $(LIBCZMQ_LIBS)

bin_PROGRAMS += 	simple server-agent db-ng

server_agent_SOURCES =	src/drivers/server-agent/server-agent.cpp

server_agent_LDFLAGS =	${LIBCZMQ_LIBS} ${CXXTOOLS_LIBS} ${LIBMLM_LIBS}

server_agent_LDADD =	libpriv-proto.la

server_agent_CPPFLAGS =	$(AM_CPPFLAGS)

db_ng_SOURCES =		src/persist/db-ng.cpp

db_ng_LDFLAGS =		${LIBCZMQ_LIBS} ${CXXTOOLS_LIBS} \
			${LIBMLM_LIBS} -ltntdb -lcidr

db_ng_LDADD = \
                        libpriv-db.la \
			libpriv-cidr.la \
			libpriv-log.la \
                        libpriv-proto.la

db_ng_CPPFLAGS =	$(AM_CPPFLAGS)

simple_SOURCES = \
			src/simple/main.cc \
			src/include/defs.h \
			tools/dbinit.h \
			src/persist/persistence.h \
			src/shared/upsstatus.cc \
			src/drivers/nut/nut-actor.cc \
			src/drivers/nut/nut-driver.cc

simple_LDFLAGS = \
			${LIBCZMQ_LIBS} \
			${LIBNUTCLIENT_LIBS} \
			${CXXTOOLS_LIBS} \
			-lcidr -ltntdb

simple_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_builddir)/tools/ \
			-I$(abs_top_srcdir)/tools/ \
			-I$(abs_top_srcdir)/src/persist \
			-I$(abs_top_srcdir)/src/simple/ \
			-I$(abs_top_srcdir)/src/drivers/nut

simple_LDADD = \
			libpriv-defs.la \
			libpriv-db.la \
			libpriv-cidr.la \
			libpriv-log.la \
			libpriv-proto.la \
			libpriv-subprocess.la

dist_noinst_HEADERS += \
			src/include/asset_msg.h \
			src/simple/asset_types.h \
			src/include/powerdev_msg.h \
			src/drivers/nut/nut-actor.h \
			src/drivers/nut/nut-driver.h


bin_PROGRAMS +=		driver-nmap

driver_nmap_SOURCES = \
			src/drivers/nmap/nmap-driver.h \
			src/drivers/nmap/nmap.cc

driver_nmap_LDADD = \
			libpriv-defs.la \
			libpriv-cidr.la \
			libpriv-log.la \
			libpriv-proto.la \
			libpriv-nmap-parse.la \
			libpriv-subprocess.la

driver_nmap_LDFLAGS =	${LIBCZMQ_LIBS} ${CXXTOOLS_LIBS} -lcidr

driver_nmap_CPPFLAGS =	$(AM_CPPFLAGS) \
			-I$(abs_top_srcdir)/src/simple

# Generated header needed for any programs that use the database
BUILT_SOURCES +=	tools/dbpath.h

pkglib_LTLIBRARIES += bios_web.la

# Many but not all of CPP files with bios_web.la are generated from ECPP files
# List them explicitly as separate sets, so that generated files can be cleaned
BUILT_SOURCES_bios_web_la = \
                      src/web/src/auth.cpp  src/web/src/auth-verify.cpp  \
                      src/web/src/auth-require.cpp                       \
                      src/web/src/power.cpp src/web/src/item.cpp         \
                      src/web/src/json.cpp  src/web/src/list.cpp         \
                      src/web/src/time.cpp  src/web/src/sysinfo.cpp      \
                      src/web/src/network.cpp src/web/src/networks.cpp   \
                      src/web/src/current.cpp src/web/src/ui_properties.cpp  \
                      src/web/src/rack_total.cpp src/web/src/location.cpp    \
                      src/web/src/datacenter_indicators.cpp              \
                      src/include/git_details_override.h

bios_web_la_SOURCES = $(BUILT_SOURCES_bios_web_la)                       \
                      src/web/src/location_helpers.cpp                   \
                      src/web/src/mock.cpp  src/web/src/sasl.cpp         \
                      src/web/src/tokens.cpp src/shared/data.cpp         \
                      src/shared/upsstatus.cc

# Ensure these files are auto-made before others
BUILT_SOURCES +=	$(BUILT_SOURCES_bios_web_la)

# Ensure these files are cleaned away too
CLEANFILES +=		$(BUILT_SOURCES_bios_web_la)

bios_web_la_CPPFLAGS =	$(AM_CPPFLAGS) \
			${LIBCZMQ_CFLAGS} ${CXXTOOLS_CFLAGS} \
			${LIBSODIUM_CFLAGS} ${LIBSASL2_CFLAGS} \
			-DSASLAUTHD_MUX=${SASLAUTHD_MUX} \
			-I$(abs_top_srcdir)/src/simple \
			-I$(abs_top_srcdir)/src/web/include \
			-I$(abs_top_builddir)/tools \
			-I$(abs_top_srcdir)/tools \
			-I$(abs_top_srcdir)/src/persist

bios_web_la_LDFLAGS = 	${LIBCZMQ_LIBS}	${CXXTOOLS_LIBS} \
			${LIBSODIUM_LIBS} ${LIBSASL2_LIBS} \
			-module -lm -ltntdb -ltntnet -lcidr
bios_web_la_LIBADD = \
			libpriv-db.la libpriv-cidr.la libpriv-proto.la \
			libpriv-git-details-override.la \
			libpriv-log.la libpriv-subprocess.la

dist_noinst_HEADERS += \
			src/shared/data.h \
			src/web/include/sasl.h \
			src/web/include/tokens.h \
			src/persist/assetcrud.h \
			tools/dbpath.h

EXTRA_DIST +=		src/web/tntnet.xml

### This file contains the TNTNET settings relevant for our test runs
### Note: in non-daemon mode, stderr is logged to stderr
tntnet.xml: src/web/tntnet.xml
	${SED} -e 's|\(.*\)\(<!--.*<dir>/</dir>.*-->.*\)|\1<dir>$(abs_top_srcdir)/src/web</dir>\n\1<compPath><entry>$(abs_top_builddir)/.libs</entry></compPath>|' $< > $@ || \
	rm -f $@

### This file contains the TNTNET settings distributed as an example for users
### Note: in daemon mode, "stderr > /dev/null" unless errorLog is defined
tntnet.xml.example: src/web/tntnet.xml
	${SED} -e 's|\(.*\)\(<!--.*<dir>/</dir>.*-->.*\)|\1<dir>$(datarootdir)/@PACKAGE@-@VERSION@/web</dir>\n\1<compPath>\n\1\ \ \ <entry>$(pkglibdir)</entry>\n\1\ \ \ <entry>$(libdir)</entry>\n\1</compPath>|' \
	       -e 's|<!-- <errorLog>/var/log/tntnet/error.log</errorLog> -->|<errorLog>/var/log/tntnet/error.log</errorLog>|' \
	       -e 's|<!--\ <daemon>0</daemon>\ -->|<daemon>1</daemon>|' $< > $@ || \
	rm -f $@

exampleconfdir =	$(datarootdir)/@PACKAGE@-@VERSION@/examples
exampleconf_DATA =	tntnet.xml.example
EXTRA_DIST +=		$(exampleconf_DATA)

CLEANFILES +=		tntnet.xml tntnet.xml.example

# Make sure needed files are up-to-date to test web-server for $BIOS REST API
web-test-deps: tntnet.xml bios_web.la

# Execute the test web-server for $BIOS REST API
web-test: web-test-deps
	@echo "=== Running target $@ as: "; id || true; echo ""
	LC_ALL=C LANG=C ${TNTNET} tntnet.xml

#----------------------------------------------------------------------
#                        Extra files and sources
#----------------------------------------------------------------------

EXTRA_DIST +=		$(top_srcdir)/tools/gen-json-validator \
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
EXTRA_DIST +=		$(top_srcdir)/tests/CI

# This is complicated... who knows why...
EXTRA_DIST += \
			$(top_srcdir)/tools/dbpath.h.in \
			$(top_srcdir)/tools/dbinit.h

DISTCLEANFILES +=	tools/dbpath.h

#BUILT_SOURCES +=	$(abs_top_srcdir)/tools/dbinit.h

#----------------------------------------------------------------------
#           list of text-based documentation files (rules below)
# Note that the basic filenames in these variables are relative to the
# project sources' root directory, but later in usage and actual targets
# they are prefixed by the build directory (which may or may not be the
# same as the checked-out source directory, both ways SHOULD work).
#----------------------------------------------------------------------

# For legacy/simplicity several types of file list variables are defined
# below to reference the manually maintained and repository-tracked files
# with documentation in text format, including the sources for automated
# conversion from asciidoc to html (i.e. not doxygen, not manpages).
# See comments below for path/extension expectations for different file types.
# Variables below include:
#
# Auto-generated:
#  TEXTS - list of *.txt files that are delivered by the project. Generated
#	from MAPPED_TXT_DOCS_* lists. Required by "docs-txt" target.
#  HTMLS_ASCIIDOC - list of files generated automatically from asciidoc
#	into HTML (1:1 relation, same dirname/basename, changes extension);
#	the list is generated from MAPPED_TXT_DOCS_ASCIIDOC.
#  HTMLS - list of *.html files (except doxygen) that are delivered by the
#	project (built via target rules if needed); currently this only
#	includes HTMLS_ASCIIDOC. Required by "docs-html" target.
#  CLEAN_TEXTS and CLEAN_HTMLS - list of files generated/copied by dynamic
#	rules lower in this Makefile, so they can be cleaned up too.
#
# Half-manual, half automation:
#  MAPPED_TXT_DOCS_PLAINTEXT and MAPPED_TXT_DOCS_ASCIIDOC - lists of all
#	colon-separated couples of source and destination text filenames
#	which may involve a copy to another directory, another naming,
#	changes of extension, etc. Populated manually and then from the
#	other lists (BASE_DOCS_*, TXT_DOCS_*) with simple deterministic
#	conversion rules from list to mapping entry. These mappings are
#	used to dynamically determine make-recipes and targets for files.
#
# Component lists (populated manually, reprocessed into MAPPED_* lists);
# Note that these lists reference files tracked as part of the project
# sources (not auto-copied, named or generated as in some lists above):
#  BASE_DOCS_ASCIIDOC and BASE_DOCS_PLAINTEXT - the text files without
#	extension, saved in the root of project sources. These variables
#	are used to populate the relevant MAPPED_TXT_DOCS_* lists.
#  TXT_DOCS_ASCIIDOC and TXT_DOCS_PLAINTEXT - lists of *.txt files that
#	would actually be installed; as seen below, the list starts with
#	some files already tracked in the needed location (under docs/).
#	These variables are used to populate MAPPED_TXT_DOCS_* lists.
#
# The definition/processing order should be as follows:
# * Populate the manually tracked lists (BASE_DOCS_*, TXT_DOCS_*) and the
#   initial entries in MAPPED_TXT_DOCS_* (set those mappings which rename
#   text files from tracked source to delivery under "docs/develop/").
# * Define as empty (or very exceptionally pre-populated) the lists in
#   HTMLS_ASCIIDOC, HTMLS, TEXTS as well as automated installation targets
#   like develdoc_DATA.
# * Run macros to populate the rest of MAPPED_TXT_DOCS_* with trivial
#   conversions from BASE_DOCS_* and TXT_DOCS_*.
# * Run macros to define the build targets and recipes for all text files
#   (copying) and add to our targets (TEXTS) and other common lists like
#   EXTRA_DIST and DISTCLEAN automatically.
# * Run macros to define the build targets and recipes for the HTML files
#   created from texts by asciidoc (append to HTMLS_ASCIIDOC).
# * Add the resulting HTMLS_ASCIIDOC to HTMLS.
# * Define the practical targets - to build or clean the whole set of our
#   documentation files.

# Documentation files saved in the project source-code root directory
# as text files without extensions in the name (following GNU standards)
# and written in asciidoc markup
# Note these should have paths relative to project root dir.
BASE_DOCS_ASCIIDOC =	INSTALL INSTALL-referenceOS-debian8 \
			CONTRIBUTING 

# Documentation files saved in the project source-code root directory
# as text files without extensions in the name and written in plain
# text with no expected transformation
# Note these should have paths relative to project root dir.
BASE_DOCS_PLAINTEXT =	AUTHORS COPYING NEWS ChangeLog

# These two seem to have some markup or similar structuring,
# but it is not asciidoc so consider them plaintext for now.
# Note these should have paths relative to project root dir.
BASE_DOCS_PLAINTEXT +=	README
BASE_DOCS_PLAINTEXT +=	TODO

# The following files are saved into proper subdirectories, have an
# extension in the filename, and contain asciidoc markup.
# Note these should have paths relative to project root dir.
TXT_DOCS_ASCIIDOC = 	$(myDEVDOCDIR)/README-builder.txt \
			$(myDEVDOCDIR)/README-init-os-accounts.txt

# List of text files that are installed as is (none so far, but see
# below for MAPPED_TXT_DOCS_ASCIIDOC and MAPPED_TXT_DOCS_PLAINTEXT)
TXT_DOCS_PLAINTEXT =


#----------------------------------------------------------------------
#  List of source texts dispersed in the code with automated targeting
#----------------------------------------------------------------------
# The list below is a colon-separated map of filenames copied from
# sources relative to project root (first token) into different names
# under the builddir and final installation (the second token, here
# copied into "docs/develop"), and also maybe compiled into HTML
# (as in case of the _ASCIIDOC list).
# Filenames here should not have spaces and should have the '.txt' suffix.

MAPPED_TXT_DOCS_PLAINTEXT = \
	src/simple/README:$(myDEVDOCDIR)/README-archdoc-simplePOC.txt

MAPPED_TXT_DOCS_ASCIIDOC = \
	tests/CI/web/README.txt:$(myDEVDOCDIR)/README-web-tests-ciAutomation.txt \
	src/web/README.txt:$(myDEVDOCDIR)/README-web-tests-generalInfo.txt \
	src/drivers/nut/README:$(myDEVDOCDIR)/README-drivers-nut.txt

if ENABLE_DOCKER_SUPPORT
MAPPED_TXT_DOCS_ASCIIDOC += \
	docker/README:$(myDEVDOCDIR)/README-docker.txt
endif

#################################################################
# Items below are intended for automatic generation and appendage
#################################################################
# Historically some original docs have *.txt names and some don't
# but we solve it by having a target to ensure presence of *.txt
# copies now for all needed files in the same docs directory
# See the MAPPED_TXT_DOCS_ASCIIDOC parsing around this Makefile.
# Here we predefine the HTMLS_ASCIIDOC list as anything that is
# not converted from the MAPPED_TXT_DOCS_ASCIIDOC list, so empty.
HTMLS_ASCIIDOC =

# Ultimate list of HTMLs from generally many sources (except doxygen,
# since that list of targets is not known in advance). Predefined (and
# empty) here, appended later on. Similarly for ultimate text files.
HTMLS =
TEXTS =

# These variables track the compiled/generated/etc. text/html files
# so they can be removed in *clean requests
CLEAN_HTMLS =
CLEAN_TEXTS =

##################################################
# Define delivery of developer documentation:
##################################################
# Target directory for developer documentation after installation
develdocdir = $(datadir)/@PACKAGE@-@VERSION@/develop

# Install the non-doxygen docs here (doxygen has its subdir defined below).
# Text files are "originals" or copies under a different name and are always
# expected to be available (to fail the make otherwise is correct action).
# Note that other HTML component targets must be listed explicitly as they
# might appear in the project later on; as for example HTMLS_ASCIIDOC needs
# a conditional enablement (try install only if generatable at all).
# Like above, this list is initially empty (or maybe explicit later on)
# and is appended by some macros below.
develdoc_DATA =

###----------------------------------------------------------------------
###   Magic macros to expand the patterns for text to text file copying
###----------------------------------------------------------------------
# Magic below inspired by this blog post (see description/comments):
#   http://blog.jgc.org/2012/01/using-gnu-makes-define-and-eval-to.html

# Copies from "$1" to "$2", making the subdirectories as needed
define copy-file-cp =
	if test x"$(2)" != x"$(1)" ; then \
	    { test -d "`dirname "$(2)"`" || mkdir -p "`dirname "$(2)"`"; } && \
	    echo "Copy '$(1)' to '$(2)'" && \
	    cp -pf "$(1)" "$(2)" || cp -f "$(1)" "$(2)"; \
	else \
	    echo "Not copying '$(1)' to '$(2)': same file"; \
	fi
endef

# Another way to skin the cat...
define copy-file-install =
	$(INSTALL) -D -T -p -v -m 644 "$1" "$2"
endef

# Generalize the two solutions
define copy-file =
	$(call copy-file-install,$1,$2) || $(call copy-file-cp,$1,$2)
endef

# This macro defines the dependencies and rules to copy the text file
# into the build area. Also adds the filename into lists for install,
# cleanup, ultimately used dependencies, etc.
define depend-copy-file =
$(eval override _MAP_SRC_REL :=	$(firstword $(subst :, ,$(1))) )
$(eval override _MAP_DST_REL :=	$(word 2,$(subst :, ,$(1))) )
$(eval override _MAP_SRC_ABS :=	$(addprefix $(abs_top_srcdir)/,$(_MAP_SRC_REL)) )
$(eval override _MAP_DST_ABS :=	$(addprefix $(abs_top_builddir)/,$(_MAP_DST_REL)) )

# Define the dependency and the rule
$(_MAP_DST_ABS): $(_MAP_SRC_ABS)
	@$$(call copy-file,$$<,$$@)

# Fake dependency for subdir/dist builds
$(_MAP_DST_REL): $(_MAP_DST_ABS)
	@#echo "DEBUG-MAKEFILE-FAKEDEP: Mapped target '$$@' to source '$$<'"

### Target as a text file
$(eval TEXTS +=			$(_MAP_DST_REL) )

### Copy for installation:
$(eval develdoc_DATA +=		$(_MAP_DST_REL) )

### Copy in make dist
$(eval EXTRA_DIST +=		$(_MAP_SRC_REL) )
endef

define canclean-txt-file =
$(eval override _MAP_SRC_REL :=	$(firstword $(subst :, ,$(1))) )
$(eval override _MAP_DST_REL :=	$(word 2,$(subst :, ,$(1))) )

### Cleanup in build
$(eval CLEANFILES +=		$(_MAP_DST_REL) )
$(eval CLEAN_TEXTS +=		$(_MAP_DST_REL) )
endef

# Convert the simple lists defined above into the common mapping tables
# for unprocessable and for asciidoc-able text files:

$(foreach m,$(BASE_DOCS_ASCIIDOC), \
	$(eval MAPPED_TXT_DOCS_ASCIIDOC += ${m}:${myDEVDOCDIR}/${m}.txt ))

$(foreach m,$(BASE_DOCS_PLAINTEXT), \
	$(eval MAPPED_TXT_DOCS_PLAINTEXT += ${m}:${myDEVDOCDIR}/${m}.txt ))

# Add to automatic removal ONLY the files which are relocated while mapping
# That is, TXT_DOCS_* which remain in-place should not be removed (i.e. the
# originals during an in-tree build)

$(foreach m,$(MAPPED_TXT_DOCS_PLAINTEXT) $(MAPPED_TXT_DOCS_ASCIIDOC), \
	$(eval $(call canclean-txt-file,$(m))))

# Convert the 1:1 simple lists defined above into the common mapping tables
# for unprocessable and for asciidoc-able text files:

$(foreach m,$(TXT_DOCS_ASCIIDOC), \
	$(eval MAPPED_TXT_DOCS_ASCIIDOC += ${m}:${m} ))

$(foreach m,$(TXT_DOCS_PLAINTEXT), \
	$(eval MAPPED_TXT_DOCS_PLAINTEXT += ${m}:${m} ))

# Finally, run the logic for patterns that define file-copying
# Note that for 1:1 mapped files there may be harmless messages like
# "make: Circular...dependency dropped." during an in-directory build.
$(foreach m,$(MAPPED_TXT_DOCS_PLAINTEXT) $(MAPPED_TXT_DOCS_ASCIIDOC), \
	$(eval $(call depend-copy-file,$(m))))



##################################################
# The stuff HTML docs are made of...
# Similar activity for ASCIIDOC-HTML mapping
# The dependencies for the text file via patterns in the standard dir
# were defined above. Now just add the dependency for HTML building
# into the same basename...
# Per dependencies, all the text files should be available in the
# builddir/docs/develop/*.txt and the *.html will appear nearby

define compile-asciidoc-to-html =
if HAVE_ASCIIDOC
	@test -d "`dirname $(2)`" || mkdir -p "`dirname $(2)`"
	$(ASCIIDOC) -b html -o $(2) $(1)
else
	@echo "SKIP: Missing 'asciidoc' program, generic HTML documentation generation skipped for '$(1)' -> '$(2)'!" >&2
endif
endef

define depend-txt-html =
$(eval override _MAP_DST_REL :=		$(word 2,$(subst :, ,$(1))) )
$(eval override _HTML_DST_REL :=	$(_MAP_DST_REL:%.txt=%.html) )
$(eval override _MAP_DST_ABS :=		$(addprefix $(abs_top_builddir)/,$(_MAP_DST_REL)) )
$(eval override _HTML_DST_ABS :=	$(addprefix $(abs_top_builddir)/,$(_HTML_DST_REL)) )

# Define the dependency and the rule
$(_HTML_DST_ABS): $(_MAP_DST_ABS)
	@$$(call compile-asciidoc-to-html,$$<,$$@)

# Fake dependency for subdir/dist builds
$(_HTML_DST_REL): $(_HTML_DST_ABS)
	@#echo "DEBUG-MAKEFILE-FAKEDEP: Mapped target '$$@' to source '$$<'"

$(eval CLEAN_HTMLS +=		$(_HTML_DST_REL) )

$(eval HTMLS_ASCIIDOC +=	$(_HTML_DST_REL) )
endef

# Some HTMLS_ASCIIDOCs may have been defined above, maybe with other recipes
# Convert the remainder of HTMLS_ASCIIDOC from MAPPED_TXT_DOCS_ASCIIDOC
$(foreach m,$(MAPPED_TXT_DOCS_ASCIIDOC), $(eval $(call depend-txt-html,$(m))))


if HAVE_ASCIIDOC
HTMLS +=		$(HTMLS_ASCIIDOC)
develdoc_DATA +=	$(HTMLS_ASCIIDOC)
CLEANFILES +=		$(HTMLS_ASCIIDOC)
endif



#----------------------------------------------------------------------
#                One rule to generate and wipe documentation
#----------------------------------------------------------------------

all-docs: man-docs doxygen-docs html-docs txt-docs

# alias for manual typing errors
docs-all: all-docs

clean-docs: clean-man clean-doxygen clean-html clean-txt


#----------------------------------------------------------------------
#                        text to text copying
# Some documents are by GNU standard stored in project root, and we
# want to deliver them to common docs/devel subdirectory in install.
#----------------------------------------------------------------------

# A copy of TXT_DOCS_ASCIIDOC in the build area is needed for unified
# rules of asciidoc to html parsing, defined below; see MAPPED_* above.
txt-docs: $(addprefix $(abs_top_builddir)/,$(TEXTS))

# alias for manual typing errors
docs-txt: txt-docs

clean-txt: clean-txt-built clean-txt-txt_docs

clean-txt-built:
	/bin/rm -f $(addprefix $(abs_top_builddir)/,$(CLEAN_TEXTS))

# Only clean up the copies in the builddir, if applicable
# Do not kill the originals in the source tree
clean-txt-txt_docs:
	if test x"$(abs_top_srcdir)" != x"$(abs_top_builddir)" -a \
	    x"$(abs_top_srcdir)" != x -a x"$(abs_top_builddir)" != x ; then \
	    /bin/rm -f $(addprefix $(abs_top_builddir)/,$(TXT_DOCS_ASCIIDOC) $(TXT_DOCS_PLAINTEXT)); \
	else true; fi


#----------------------------------------------------------------------
#                       asciidoc html generation
#----------------------------------------------------------------------

# Technically we only implement building of HTMLS_ASCIIDOC now.
# If others appear and are not handled, they will fail to make.
# That is a good thing by design - we'll know to add more logic ;)


html-docs: txt-docs $(addprefix $(abs_top_builddir)/,$(HTMLS))

# alias for manual typing errors
docs-html: html-docs

clean-html:
	/bin/rm -f $(addprefix $(abs_top_builddir)/,$(CLEAN_HTMLS))


#----------------------------------------------------------------------
#                        doxygen html generation
# Note that unlike many other targets, doxygen seemingly has to use
# relative paths and be generated in a directory near the source code 
#----------------------------------------------------------------------

if HAVE_DOXYGEN
EXTRA_DIST +=		$(top_srcdir)/${myDOXDIR}/html
BUILT_SOURCES +=	$(top_srcdir)/${myDOXDIR}/html
endif
EXTRA_DIST +=		$(top_srcdir)/${myDOXDIR}/Doxyfile.in

CLEANFILES +=		$(top_srcdir)/${myDOXDIR}/doxygen_sqlite3.db \
			$(top_srcdir)/${myDOXDIR}/${mydoxylog} \
			${myDOXDIR}/doxygen_sqlite3.db \
			${myDOXDIR}/${mydoxylog}

$(top_srcdir)/${myDOXDIR}/html: $(abs_top_srcdir)/${myDOXDIR}/html
doxygen-docs: $(abs_top_srcdir)/${myDOXDIR}/html

# alias for manual typing errors
docs-doxygen: doxygen-docs

clean-doxygen:
	( test -w $(abs_top_srcdir)/${myDOXDIR}/html && \
	  /bin/rm -rf $(abs_top_srcdir)/${myDOXDIR}/html ) || /bin/true

# Copy generated contents of "docs/doxygen/" into "/usr/share/core.../doxygen"
develdocdoxygendir =	$(develdocdir)/doxygen
develdocdoxygen: doxygen-docs

if HAVE_DOXYGEN
develdocdoxygen_DATA =	${myDOXDIR}/html/*

# Such-named target is required by the _DATA above, so have a rule to build it
${myDOXDIR}/html/*: doxygen-docs

$(abs_top_srcdir)/${myDOXDIR}/html: $(abs_top_builddir)/${myDOXDIR}/Doxyfile
	@test -d $(abs_top_srcdir)/${myDOXDIR}/html || \
	{ cd $(abs_top_srcdir)/ && \
	  echo "INFO: Generating Doxygen docs in '`pwd`' according to '$<'" >&2 && \
	  $(DOXYGEN) "$<"; }
else
develdocdoxygen_DATA =

${myDOXDIR}/html/*: $(abs_top_srcdir)/${myDOXDIR}/html

$(abs_top_srcdir)/${myDOXDIR}/html:
	@echo "SKIP: Missing 'doxygen' program, sourcecode-documentation generation skipped!" >&2
endif

#----------------------------------------------------------------------
#                 manpage generation (delegated to sub-make)
#----------------------------------------------------------------------

EXTRA_DIST +=		$(top_srcdir)/${myMANDIR}/*.txt
CLEANFILES +=		$(abs_top_builddir)/${myMANDIR}/*.?

man-docs:
	( cd $(abs_top_builddir)/$(myMANDIR) && $(MAKE) all )

# alias for manual typing errors
docs-man: man-docs

clean-man:
	( cd $(abs_top_builddir)/$(myMANDIR) && \
	  { $(MAKE) clean || /bin/true; } && \
	  /bin/rm -f *.[123456789] *.xml *.html )
	/bin/rm -f $(abs_top_builddir)/$(myMANDIR)/*.?

#----------------------------------------------------------------------
#                        hook for make all => generate files too
#----------------------------------------------------------------------

all-local: $(abs_top_builddir)/.git_details all-docs


#----------------------------------------------------------------------
#                        clean, what was not removed automatically
#----------------------------------------------------------------------

clean-junit:
	/bin/rm -rf $(abs_top_builddir)/tests/junit

clean-local: clean-docs clean-junit

# This may and should complain on non-empty dirs; "distclean-generic"
# with proper list of DISTCLEANFILES should have taken care of that
distclean-local-dirs: distclean-generic
	for D in $(addprefix $(abs_top_builddir)/,$(DISTCLEANDIRS)); do \
	  [ ! -d "$$D" ] || /bin/rmdir "$$D"; done || true

distclean-local-deps: distclean-generic
	cd $(abs_top_builddir)/ && { \
	  find . -type f -name '*.Plo' -exec /bin/rm -f '{}' \; ; \
	  find . -type f -name '*.Po' -exec /bin/rm -f '{}' \; ; \
	  find . -type d -name '.deps' -exec /bin/rmdir '{}' \; || true ; \
	}

# BUILDER_RETAIN_CONFIGURE=yes may be exported by builder.sh
distclean-local: distclean-local-dirs distclean-local-deps
	@if [ x"$(BUILDER_RETAIN_CONFIGURE)" = xyes ]; then \
	    echo "INFO: Retaining the configure script because I was asked to"; \
	else \
	    echo "INFO: Removing the configure script as part of distclean"; \
	    /bin/rm -f $(abs_top_builddir)/configure; \
	fi >&2 || true
	@if [ x"$(abs_top_builddir)/.git_details" != x"$(abs_top_srcdir)/.git_details" ]; then \
	    /bin/rm -f $(abs_top_builddir)/.git_details; \
	fi || true


#----------------------------------------------------------------------
#                        astyle formating
#----------------------------------------------------------------------
if HAVE_ASTYLE
EXTRA_DIST +=		$(top_srcdir)/tools/astyle.conf \
			$(top_srcdir)/tools/astyle-bios
checkstyle:
	$(abs_top_srcdir)/tools/astyle-bios \
	    -c $(abs_top_srcdir)/tools/astyle.conf $(abs_top_srcdir)/src
style:
	$(abs_top_srcdir)/tools/astyle-bios \
	    $(abs_top_srcdir)/tools/astyle.conf $(abs_top_srcdir)/src
endif

#----------------------------------------------------------------------
#                        some popular testing targets
#----------------------------------------------------------------------

ci-test-restapi: test-restapi
test-restapi: all
	$(abs_top_srcdir)/tests/CI/ci-test-restapi.sh


#----------------------------------------------------------------------
#                        SCM details
#----------------------------------------------------------------------

EXTRA_DIST +=		.git_details \
			tools/git_details.sh \
			tools/JSON.sh

CLEANFILES +=		src/include/git_details_override.c \
			src/include/git_details_override.h

# The .git_details can be generated by OBS script which does the checkout of
# sources to be packaged, or by the top-level make in a "make dist" running
# in a Git workspace. Namely, this might not be regeneratable in each case,
# i.e. when just a tarball with no SCM data is used for the build - then the
# tarball should contain a current (latest generated) copy of this file.

# This target ensures the .git_details file is always re-evaluated somehow
.PHONY_GIT:

.git_details: $(abs_top_builddir)/.git_details

# NOTE that in the target routine below we filter out PACKAGE_BUILD_TSTAMP
# changes because if nothing else changed, these don't matter much but do
# cause a rebuild of the C/H files and those which depend on them, on every
# "make" run. Just remove the file (distclean) to regenerate everything truly.
$(abs_top_builddir)/.git_details: .PHONY_GIT
	@echo "INFO: Checking that '$@' is up-to-date..."
	@/bin/rm -f "$@.tmp"
	@if ( cd $(abs_top_srcdir) && ./tools/git_details.sh ) > "$@.tmp" && \
	    [ -s "$@.tmp" ]; then \
	    if [ ! -s "$@" ] || diff -bu "$@" "$@.tmp" | \
	        egrep -v '^(\-\-\-|\+\+\+|[ @]|[\+\-]PACKAGE_BUILD_TSTAMP)'; then \
	      echo "INFO: Replacing '$@' with newly generated data" && \
	      mv -f "$@.tmp" "$@"; \
	    else echo "INFO: No substantial changes in contents of '$@' (although ignored PACKAGE_BUILD_TSTAMP)"; \
	    fi; \
	 else \
	   [ -s "$@" ] && echo "INFO: Reusing existing '$@'" && ls -la "$@"; \
	 fi || true
	@[ -s "$@" ] || \
	    { [ -s  "$(abs_top_srcdir)/.git_details" -a \
	           x"$(abs_top_srcdir)/.git_details" != x"$@" ] && \
              cp -f "$(abs_top_srcdir)/.git_details" "$@" ; }
	@/bin/rm -f "$@.tmp"
	@[ -s "$@" ] || { echo "WARNING: Overriding absent '$@' with empty file to satisfy 'make'"; touch "$@"; }

src/include/git_details_override.c: $(abs_top_builddir)/src/include/git_details_override.c

$(abs_top_builddir)/src/include/git_details_override.c: $(abs_top_builddir)/.git_details
	@echo "INFO: Checking that '$@' is up-to-date..."
	@( echo '// WARNING: This file is generated during make, DO NOT EDIT'; \
	  echo '#include <src/include/git_details_override.h>'; \
	  grep '=' "$<" | sed 's,^\(PACK.*;\)$$,char * \1,'; ) | sed 's,\\,\\\\,g' > "$@.tmp" || \
	 /bin/rm -f "$@.tmp"
	@{ [ -s "$@.tmp" ] && { diff "$@" "$@.tmp" >/dev/null 2>/dev/null && \
	    echo "INFO: No changes in contents of '$@'" || \
	    mv -f "$@.tmp" "$@"; } } || { \
	  [ -s "$@" ] && echo "INFO: Reusing existing '$@'" && ls -la "$@"; }; \
	  true
	@/bin/rm -f "$@.tmp"
	@[ -s "$@" ] || { echo "WARNING: Overriding absent '$@' with empty file to satisfy 'make'"; touch "$@"; }

src/include/git_details_override.h: $(abs_top_builddir)/src/include/git_details_override.h

$(abs_top_builddir)/src/include/git_details_override.h: $(abs_top_builddir)/.git_details
	@echo "INFO: Checking that '$@' is up-to-date..."
	@( echo '// WARNING: This file is generated during make, DO NOT EDIT'; \
	  echo '#ifndef BIOS_GIT_DETAILS_OVERRIDE' && \
	  echo '#define BIOS_GIT_DETAILS_OVERRIDE' && \
	  grep '=' "$<" | sed 's,^\(PACK[^=]*\)=.*$$,#undef \1,' && \
	  grep '=' "$<" | sed 's,^\(PACK[^=]*\)=.*$$,extern char * \1;,' && \
	  echo '#endif // BIOS_GIT_DETAILS_OVERRIDE'; ) > "$@.tmp" || \
	 /bin/rm -f "$@.tmp"
	@{ [ -s "$@.tmp" ] && { diff "$@" "$@.tmp" >/dev/null 2>/dev/null && \
	    echo "INFO: No changes in contents of '$@'" || \
	    mv -f "$@.tmp" "$@"; } } || { \
	  [ -s "$@" ] && echo "INFO: Reusing existing '$@'" && ls -la "$@"; }; \
	  true
	@/bin/rm -f "$@.tmp"
	@[ -s "$@" ] || { echo "WARNING: Overriding absent '$@' with empty file to satisfy 'make'"; touch "$@"; }

