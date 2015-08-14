# Makefile for docker/ to install some of the scripts and resource files for
# the end-user image. Note that the README installation and compilation is
# handled by the master Makefile.
EXTRA_DIST =

dockerdir	= $(datadir)/@PACKAGE@/docker
docker_SCRIPTS	= 	docker_build_bios.sh  \
			docker_remove.sh \
			docker_run_bios.sh \
			docker_stop.sh

dockerresdir	= $(dockerdir)/resources
dockerres_SCRIPTS =	resources/start_bios.sh
dockerres_DATA = 	resources/Dockerfile \
			resources/patch.sql \
			resources/ePDU1-LAB.dev \
			resources/ePDU2-LAB.dev \
			resources/UPS1-LAB.dev \
			resources/UPS2-LAB.dev \
			resources/ROZ.UPS1.dev \
			resources/ROZ.ePDU04.dev \
			resources/ROZ.ePDU05.dev \
			resources/ups.conf \
			resources/upsmon.conf \
			resources/interfaces \
			resources/license

resources/license: $(top_srcdir)/COPYING
	/bin/cp $(top_srcdir)/COPYING resources/license

EXTRA_DIST += $(docker_SCRIPTS) $(dockerres_SCRIPTS) $(dockerres_DATA)

