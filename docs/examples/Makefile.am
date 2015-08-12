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

EXTRA_DIST += $(examplecfgsudo_DATA) $(examplecfgpam_DATA) $(examplecfgsec_DATA)
