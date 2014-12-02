# Description: The Makefile (automake template) for the $BIOS project.
# Note: relies on GNU make syntax and features, may fail to work in
# other make programs.

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

# Named by analogy from configure'd myDOXDIR for doxygen
myMANDIR = docs/man
myDEVDOCDIR = docs/develop

# Locations of child Makefiles
SUBDIRS = $(myMANDIR) docs/examples tools

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

libpriv_subprocess_la_LIBADD = -lcxxtools

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
				src/persist/measure_types.h \
				src/include/common_msg.c \
				src/include/common_msg.h \
				src/include/nmap_msg.h \
				src/include/nmap_msg.c \
				src/include/netdisc_msg.c \
				src/include/netdisc_msg.h \
				src/include/powerdev_msg.c \
				src/include/powerdev_msg.h \
				src/include/asset_msg.c \
                                src/include/asset_msg.h

libpriv_db_la_CPPFLAGS = $(AM_CPPFLAGS) \
					-I$(top_srcdir)/src/include \
					-I$(top_srcdir)/src/persist \
					-I$(top_srcdir)/src/shared \
					-I$(top_srcdir)/src/simple \
					-I$(top_srcdir)/tools \
					-I$(top_builddir)/tools

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
test_subprocess_LDFLAGS =		-lcxxtools -lcidr

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

test_db_LDFLAGS =			-ltntdb -lcxxtools -lcidr

check_PROGRAMS += 			test-db2

test_db2_SOURCES = tests/persist/test-monitor.cc \
				   tests/persist/test-monitor-clientinfo.cc \
				   tests/persist/test-realdatamonitor.cc \
				   src/include/common_msg.h 

test_db2_LDADD = libpriv-db.la \
				 libpriv-cidr.la \
				 libpriv-test-run.la \
				 libpriv-log.la


test_db2_CPPFLAGS =	$(AM_CPPFLAGS) \
					-I$(top_srcdir)/tests/include/ \
					-I$(top_srcdir)/src/persist \
					-I$(top_srcdir)/src/simple \
					-I$(top_srcdir)/tools \
					-I$(top_builddir)/tools

test_db2_LDFLAGS =			-ltntdb -lcxxtools -lcidr -lzmq -lczmq

###
check_PROGRAMS += 			test-database

test_database_SOURCES = 	tests/persist/test-database.cc \
							src/persist/persistencelogic.cc \
							src/include/nmap_msg.h \
							src/include/nmap_msg.c

test_database_LDADD = 			libpriv-db.la \
					libpriv-cidr.la \
					libpriv-test-run.la \
					libpriv-log.la

test_database_CPPFLAGS = $(AM_CPPFLAGS) \
				   -I$(top_srcdir)/tests/include/ \
				   -I$(top_srcdir)/src/persist \
				   -I$(top_srcdir)/src/include \
				   -I$(top_srcdir)/tools \
				   -I$(top_builddir)/tools

test_database_LDFLAGS =			-ltntdb -lcxxtools -lcidr -lzmq -lczmq

###

TESTS += $(check_PROGRAMS)
TESTS += $(check_SCRIPTS)

#----------------------------------------------------------------------
#                        binaries
#----------------------------------------------------------------------

bin_PROGRAMS += 			netmon
netmon_LDFLAGS =			-lnetlink $(LIBZMQ_LIBS) \
					${LIBCZMQ_LIBS}
netmon_SOURCES = src/drivers/netmon/linux \
				 src/drivers/netmon/netmon.c \
				 src/drivers/netmon/msg_send.cc \
				 src/drivers/netmon/msg_send.h \
				 src/include/netdisc_msg.h \
				 src/include/netdisc_msg.c

netmon_CPPFLAGS = $(AM_CPPFLAGS) \
				  -I$(top_srcdir)/src/drivers/netmon \
				  -D_GNU_SOURCE \
				  -I$(top_srcdir)/src/simple

netmon_LDADD = libpriv-utils.la libpriv-log.la

bin_PROGRAMS += simple

simple_SOURCES = src/simple/main.cc \
				 src/include/defs.h \
				 $(top_srcdir)/tools/dbinit.h \
				 src/persist/persistence.h \
				 src/include/netdisc_msg.c \
				 src/include/netdisc_msg.h \
				 src/include/powerdev_msg.c \
				 src/include/common_msg.c \
				 src/include/common_msg.h \
				 src/drivers/nut/nut-actor.cc \
				 src/drivers/nut/nut-driver.cc \
				 src/include/nmap_msg.h \
				 src/include/nmap_msg.c

simple_LDFLAGS = -lcidr \
				 ${LIBCZMQ_LIBS} \
				 ${LIBNUTCLIENT_LIBS} \
				 -lcxxtools \
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
			   libpriv-subprocess.la

dist_noinst_HEADERS += src/include/asset_msg.h \
					   src/simple/asset_types.h \
					   src/include/powerdev_msg.h \
					   src/drivers/nut/nut-actor.h \
					   src/drivers/nut/nut-driver.h


bin_PROGRAMS += driver-nmap

driver_nmap_SOURCES = src/drivers/nmap/nmap-driver.h \
					  src/drivers/nmap/nmap.cc \
					  src/drivers/nmap/nmap-parse.h \
					  src/drivers/nmap/nmap-parse.cc \
					  src/include/nmap_msg.h \
					  src/include/nmap_msg.c

driver_nmap_LDADD = libpriv-subprocess.la libpriv-log.la libpriv-cidr.la
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

BUILT_SOURCES +=			$(top_srcdir)/tools/dbinit.h

#----------------------------------------------------------------------
#           list of text-based documentation files (rules below)
# Note that the basic filenames in these variables are relative to the
# project sources' root directory, but later in usage and actual targets
# they are prefixed by the build directory (which may or may not be the
# same as the checked-out source directory, both ways SHOULD work).
#----------------------------------------------------------------------


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

# These two seem to have some markup or similar structuring, but it is
# not asciidoc so consider them plaintext for now
# Note these should have paths relative to project root dir.
BASE_DOCS_PLAINTEXT +=	README
BASE_DOCS_PLAINTEXT +=	TODO

# The following files are saved into proper subdirectories, have an
# extension in the filename, and contain asciidoc markup.
# Note these should have paths relative to project root dir.
TXT_DOCS_ASCIIDOC = 	$(myDEVDOCDIR)/README-builder.txt \
			$(myDEVDOCDIR)/README-init-os-accounts.txt

# Note that the installation targets are similarly named files with
# an added ".txt" extension in the "docs/develop" subdirectory
# Note these should have paths relative to project root dir.
BASE_DOCS_TXT = 	$(BASE_DOCS_PLAINTEXT:%=$(myDEVDOCDIR)/%.txt)
BASE_DOCS_TXT +=	$(BASE_DOCS_ASCIIDOC:%=$(myDEVDOCDIR)/%.txt)

# These are also redistributed into the build area...
EXTRA_DIST +=		$(TXT_DOCS_ASCIIDOC)
EXTRA_DIST +=		$(BASE_DOCS_PLAINTEXT) $(BASE_DOCS_ASCIIDOC)

# As these files are "generated" (copied in this case),
# we may want a way to kill them from build area)
DISTCLEANFILES +=	$(BASE_DOCS_TXT)


##################################################
# The stuff HTML docs are made of...
# Historically some original docs have *.txt names and some don't
# but we solve it by having a target to ensure presence of *.txt
# copies now for all needed files in the same docs directory

HTMLS_ASCIIDOC =	$(BASE_DOCS_ASCIIDOC:%=$(myDEVDOCDIR)/%.html)
HTMLS_ASCIIDOC +=	$(TXT_DOCS_ASCIIDOC:%.txt=%.html)


# Ultimate list of HTMLs from generally many sources (except doxygen)
HTMLS =			$(HTMLS_ASCIIDOC)


# Target directory for developer documentation after installation
develdocdir = $(datadir)/@PACKAGE@-@VERSION@/develop

# Install the non-doxygen docs here (doxygen has its subdir defined below).
# Text files are "originals" or copies under a different name and are always
# expected to be available (to fail the make otherwise is correct action).
# Note that other HTML component targets must be listed explicitly as they
# might appear in the project later on; as for example HTMLS_ASCIIDOC needs
# a conditional enablement (try install only if generatable at all).
develdoc_DATA =		$(TXT_DOCS_ASCIIDOC) $(BASE_DOCS_TXT)

if HAVE_ASCIIDOC
develdoc_DATA +=	$(HTMLS_ASCIIDOC)
DISTCLEANFILES +=	$(HTMLS_ASCIIDOC)
endif

#----------------------------------------------------------------------
#                One rule to generate and wipe documentation
#----------------------------------------------------------------------

all-docs: man-docs doxygen-docs html-docs txt-docs

clean-docs: clean-man clean-doxygen clean-html clean-txt

#----------------------------------------------------------------------
#                        text to text copying
# Some documents are by GNU standard stored in project root, and we
# want to deliver them to common docs/devel subdirectory in install.
#----------------------------------------------------------------------

# A copy of TXT_DOCS_ASCIIDOC in the build area is needed for unified
# rules of asciidoc to html parsing, defined below
txt-docs: $(addprefix $(abs_top_builddir)/,$(BASE_DOCS_TXT)) \
	$(addprefix $(abs_top_builddir)/,$(TXT_DOCS_ASCIIDOC))

clean-txt: clean-txt-basedocs clean-txt-asciidocs

clean-txt-basedocs:
	/bin/rm -f $(addprefix $(abs_top_builddir)/,$(BASE_DOCS_TXT))

# Only clean up the copies in the builddir, if applicable
# Do not kill the originals in the source tree
clean-txt-asciidocs:
	if test x"$(abs_top_srcdir)" != x"$(abs_top_builddir)" -a \
	    x"$(abs_top_srcdir)" != x -a x"$(abs_top_builddir)" != x ; then \
	    /bin/rm -f $(addprefix $(abs_top_builddir)/,$(TXT_DOCS_ASCIIDOC)); \
	else true; fi

# Copy over the *.txt files into symmetrically named build path,
# if required and not there (messed up with EXT_DIST)
$(addprefix $(abs_top_builddir)/,$(TXT_DOCS_ASCIIDOC)): $(abs_top_builddir)/%.txt: $(abs_top_srcdir)/%.txt
	if test x"$@" != x"$<" ; then \
		echo "Copy '$<' to '$@'"; cp -pf "$<" "$@"; \
	fi

# This is hardcoded to copy documents from project root directory into
# the build area's "docs/develop" subdirectory and with ".txt" suffix
$(addprefix $(abs_top_builddir)/,$(BASE_DOCS_TXT)): $(abs_top_builddir)/$(myDEVDOCDIR)/%.txt: $(abs_top_srcdir)/%
	test -d "`dirname $@`" || mkdir -p "`dirname $@`"
	echo "Copy '$<' to '$@'"
	cp -pf "$<" "$@"

# Fake targets for simple pathnames (not needed for the same-to-same
# copies like TXT_DOCS_ASCIIDOC)
$(BASE_DOCS_TXT): $(addprefix $(abs_top_builddir)/,$(BASE_DOCS_TXT))



#----------------------------------------------------------------------
#                       asciidoc html generation
#----------------------------------------------------------------------

# Technically we only implement building of HTMLS_ASCIIDOC now.
# If others appear and are not handled, they will fail to make.
# That is a good thing by design ;)

html-docs: txt-docs $(HTMLS)

clean-html:
	/bin/rm -f $(addprefix $(abs_top_builddir)/,$(HTMLS))


# Fake targets for simple pathnames (not needed for the same-to-same
# copies like TXT_DOCS_ASCIIDOC)
$(HTMLS): $(addprefix $(abs_top_builddir)/,$(HTMLS))


if HAVE_ASCIIDOC
# Per dependencies, all the text files should be available in the
# builddir/docs/develop/*.txt and the *.html will appear nearby
$(addprefix $(abs_top_builddir)/,$(HTMLS_ASCIIDOC)): %.html: %.txt
	test -d "`dirname $@`" || mkdir -p "`dirname $@`"
	$(ASCIIDOC) -b html -o $@ $<
else
$(addprefix $(abs_top_builddir)/,$(HTMLS_ASCIIDOC)):
	@echo "SKIP: Missing 'asciidoc' program, generic HTML documentation generation skipped for '$@'!" >&2
endif


#----------------------------------------------------------------------
#                        doxygen html generation
# Note that unlike many other targets, doxygen seemingly has to use
# relative paths and be generated in a directory near the source code 
#----------------------------------------------------------------------

if HAVE_DOXYGEN
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/html
endif
EXTRA_DIST +=				$(top_srcdir)/${myDOXDIR}/Doxyfile.in

CLEANFILES +=		$(top_srcdir)/${myDOXDIR}/doxygen_sqlite3.db \
			$(top_srcdir)/${myDOXDIR}/${mydoxylog} \
			${myDOXDIR}/doxygen_sqlite3.db \
			${myDOXDIR}/${mydoxylog}

doxygen-docs: ${myDOXDIR}/html

clean-doxygen:
	( test -w $(top_srcdir)/${myDOXDIR}/html && /bin/rm -rf $(top_srcdir)/${myDOXDIR}/html ) || /bin/true

# Copy generated contents of "docs/doxygen/" into "/usr/share/core.../doxygen"
develdocdoxygendir = $(develdocdir)/doxygen
develdocdoxygen: doxygen-docs

if HAVE_DOXYGEN
develdocdoxygen_DATA = $(abs_top_srcdir)/${myDOXDIR}/html/*

# Such-named target is required by the _DATA above, so have a rule to build it
#$(abs_top_srcdir)/${myDOXDIR}/html/*: doxygen-docs

${myDOXDIR}/html:
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
develdocdoxygen_DATA =

${myDOXDIR}/html:
	@echo "SKIP: Missing 'doxygen' program, sourcecode-documentation generation skipped!" >&2
endif

#----------------------------------------------------------------------
#                 manpage generation (delegated to sub-make)
#----------------------------------------------------------------------

EXTRA_DIST +=				$(top_srcdir)/${myMANDIR}/*.txt
DISTCLEANFILES +=			$(top_builddir)/${myMANDIR}/*.?

man-docs:
	( cd $(top_builddir)/$(myMANDIR) && $(MAKE) all )

clean-man:
	( cd $(top_builddir)/$(myMANDIR) && { $(MAKE) clean || /bin/true; } && /bin/rm -f *.[123456789] *.xml *.html )
	/bin/rm -f $(top_builddir)/$(myMANDIR)/*.?

#----------------------------------------------------------------------
#                        hook for make all => generate files too
#----------------------------------------------------------------------

all-local: all-docs


#----------------------------------------------------------------------
#                        clean, what was not removed automatically
#----------------------------------------------------------------------

clean-junit:
	/bin/rm -rf $(top_builddir)/tests/junit

clean-local: clean-docs clean-junit

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
