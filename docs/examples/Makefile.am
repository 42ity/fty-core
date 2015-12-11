# Install these documents with example configurations to (typically)
# someplace under /usr/share


EXTRA_DIST =

# Ugly hack - root Makefile defines SED transformations for any .in template
%:: %.in $(abs_top_builddir)/Makefile Makefile
	cd ../.. && make `echo "$(CURDIR)/$@" | sed 's,^$(abs_top_builddir)/,,'`

examplecfgsudodir	= $(datarootdir)/@PACKAGE@/examples/config/sudoers.d
examplecfgsudo_DATA	= sudoers.d/bios_00_base sudoers.d/bios_01_citest

examplecfgpamdir	= $(datarootdir)/@PACKAGE@/examples/config/pam.d
examplecfgpam_DATA	= pam.d/bios

examplecfgsecdir	= $(datarootdir)/@PACKAGE@/examples/config/security
examplecfgsec_DATA	= security/bios_groups.conf

examplecfgupdatedir	= $(datarootdir)/@PACKAGE@/examples/config/update-rc3.d
examplecfgupdate_DATA	= \
                        update-rc3.d/02-http-proxy.conf \
                        update-rc3.d/05-image-arch.conf \
                        update-rc3.d/10-image-os-location.etn.conf \
                        update-rc3.d/20-image-kernel.etn.conf \
                        update-rc3.d/30-image-uboot.etn.conf \
                        update-rc3.d/90-local-paths.rc3.conf

EXTRA_DIST += $(examplecfgsudo_DATA) $(examplecfgpam_DATA) $(examplecfgsec_DATA) $(examplecfgupdate_DATA)
