# Description: The Makefile (automake template) for the $BIOS project public API.

#----------------------------------------------------------------------
# We present a public API in the form of a shared library and some
# headers with (hopefully) stabilized form of our communications
# protocol for the $BIOS project component applications (internal
# and third-party developments) to all communicate and interact.
# The structure is modeled somewhat after Linux Kernel sample LIBABC:
# https://git.kernel.org/cgit/linux/kernel/git/kay/libabc.git/tree/Makefile.am
# See also http://people.redhat.com/drepper/dsohowto.pdf

EXTRA_DIST =
bin_PROGRAMS =
check_PROGRAMS =
check_SCRIPTS =
check_LTLIBRARIES =
noinst_LTLIBRARIES =
noinst_SCRIPTS =
dist_noinst_DATA =
dist_noinst_HEADERS = 
BUILT_SOURCES =
CLEANFILES =
DISTCLEANFILES =
DISTCLEANDIRS =

TESTS =

# Initialize some more variables
pkgconfigdir =			$(libdir)/pkgconfig
pkgconfig_DATA =
pkginclude_HEADERS =
pkglib_LTLIBRARIES =
lib_LTLIBRARIES =

#########################################################################
###   Preprocessor/Compiler/Linker common flags
#########################################################################

AM_CPPFLAGS = \
		${my_CPPFLAGS} \
		-include $(abs_top_builddir)/config.h \
		-I$(abs_top_builddir)/include \
		-I$(abs_top_srcdir)/include \
		-I$(abs_top_builddir)/src/shared \
		-I$(abs_top_builddir)/src/include \
		-I$(abs_top_srcdir)/src/shared \
		-I$(abs_top_srcdir)/src/include \
		-I$(abs_top_builddir)/src/msg \
		-I$(abs_top_srcdir)/src/msg \
		-D__STDC_FORMAT_MACROS

AM_CFLAGS = \
		${my_CFLAGS} \
		-ffunction-sections \
		$(CFLAG_VISIBILITY) \
		-fdata-sections \
		$(LIBZMQ_CFLAGS) \
		$(LIBCZMQ_CFLAGS) \
		$(LIBVARIANT_CFLAGS)

AM_CXXFLAGS = \
		${my_CXXFLAGS} \
		-ffunction-sections \
		$(CXXFLAG_VISIBILITY) \
		-fdata-sections

AM_LDFLAGS = \
		${my_LDFLAGS} \
	 	-Wl,--gc-sections \
		-Wl,--as-needed



#----------------------------------------------------------------------
# Finally, here are the rules for                   LIBBIOSAPI
#----------------------------------------------------------------------


# Current interface number
LIBBIOSAPI_CURRENT=0
# Revision (implementation number) within current interface
LIBBIOSAPI_REVISION=0
# How many previous interfaces are still supported
LIBBIOSAPI_AGE=0

lib_LTLIBRARIES +=		libbiosapi.la

libbiosapi_la_HEADERS =
				$(top_srcdir)/include/app.h \
				$(top_srcdir)/include/bios_agent.h \
				$(top_srcdir)/include/ymsg.h

# where to install the headers on the system
libbiosapi_ladir =		$(includedir)/@PACKAGE@

libbiosapi_la_SOURCES = \
				app.c \
				bios_agent.c \
				ymsg.c \
				$(libbiosapi_la_HEADERS)

libbiosapi_la_CPPFLAGS = \
		$(AM_CPPFLAGS) -DBUILDING_LIBBIOSAPI=1

libbiosapi_la_LDFLAGS = \
		$(AM_LDFLAGS) \
		-fPIC -shared \
		${LIBZMQ_LIBS} ${LIBCZMQ_LIBS} ${LIBMLM_LIBS} \
		-version-info $(LIBBIOSAPI_CURRENT):$(LIBBIOSAPI_REVISION):$(LIBBIOSAPI_AGE)

if HAVE_LD_VERSION_SCRIPT
libbiosapi_la_LDFLAGS += \
		-Wl,--version-script=$(top_srcdir)/src/api/libbiosapi.sym
else
libbiosapi_la_LDFLAGS += \
		-export-symbols $(top_srcdir)/src/api/libbiosapi.sym
endif

libbiosapi_la_DEPENDENCIES =	$(top_srcdir)/src/api/libbiosapi.sym
EXTRA_DIST +=			libbiosapi.sym

pkgconfig_DATA +=		libbiosapi.pc
EXTRA_DIST +=			libbiosapi.pc.in  libbiosapi.pc
DISTCLEANFILES +=		libbiosapi.pc

pkginclude_HEADERS += 		$(libbiosapi_la_HEADERS)
EXTRA_DIST +=			$(libbiosapi_la_HEADERS)

$(abs_top_builddir)/libbiosapi.la: libbiosapi.la
	cp -pf "$<" "$@" || cp -f "$<" "$@"

#----------------------------------------------------------------------
# Here we would later add some program to test the libbiosapi

#TESTS +=			test-libbiosapi
#check_PROGRAMS +=		test-libbiosapi
#test_libbiosapi_SOURCES =	test-libbiosapi.c test-libbiosapi.h
#test_libbiosapi_LDADD =	$(AM_LDFLAGS) biosapi.la

