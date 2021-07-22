# Install these documents with example configurations to (typically)
# someplace under /usr/share


EXTRA_DIST =

examplecfgsudodir	= $(datarootdir)/@PACKAGE@/examples/config/sudoers.d
examplecfgsudo_DATA	= sudoers.d/fty_00_base sudoers.d/fty_01_citest

examplecfgpamdir	= $(datarootdir)/@PACKAGE@/examples/config/pam.d
examplecfgpam_DATA	= pam.d/fty pam.d/fty-password pam.d/fty-auth

examplecfgsecdir	= $(datarootdir)/@PACKAGE@/examples/config/security
examplecfgsec_DATA	= security/fty_groups.conf

examplecfgupdatedir	= $(datarootdir)/@PACKAGE@/examples/config/update-rc3.d
examplecfgupdate_DATA	= \
                        update-rc3.d/02-http-proxy.conf \
                        update-rc3.d/05-image-arch.conf \
                        update-rc3.d/10-image-os-location.etn.conf \
                        update-rc3.d/20-image-kernel.etn.conf \
                        update-rc3.d/30-image-uboot.etn.conf \
                        update-rc3.d/90-local-paths.rc3.conf

examplecfgrsyslogddir	= $(datarootdir)/@PACKAGE@/examples/config/rsyslog.d
examplecfgrsyslogd_DATA	= \
			  rsyslog.d/05-bash.conf \
			  rsyslog.d/08-ipc-remote.conf \
			  rsyslog.d/10-ipc.conf \
			  rsyslog.d/20-auditlogs-remote.conf.in

exampleprofiledir	= $(datarootdir)/@PACKAGE@/examples/config/profile.d
exampleprofile_DATA     = \
			profile.d/bash_history.sh \
			profile.d/bash_syslog.sh \
			profile.d/fty_aliases.sh \
			profile.d/fty_path.sh \
			profile.d/lang.sh

exampleudevrulesdir = $(datarootdir)/@PACKAGE@/examples/config/rules.d
exampleudevrules_DATA = \
			rules.d/90-ipc-persistent-th.rules

examplelogdir	= $(datarootdir)/@PACKAGE@/examples/config/ftylog.d
examplelog_DATA = \
                   ftylog.d/10-ftylog.cfg

examplenutserverdir	= $(systemdsystemunitdir)/nut-server.service.d
examplenutserver_DATA = \
			nut-server.service.d/configure-nut-users.conf

EXTRA_DIST += $(examplecfgsudo_DATA) $(examplecfgpam_DATA) $(examplecfgsec_DATA) $(examplecfgupdate_DATA) $(examplecfgrsyslogd_DATA) $(exampleprofile_DATA) $(exampleudevrules_DATA) $(examplelog_DATA) $(examplenutserver_DATA)
