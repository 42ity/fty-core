# Makefile for tools/ to install some of the scripts for end-user image
EXTRA_DIST =

helperdir	= $(datadir)/@PACKAGE@/scripts
helper_SCRIPTS	= init-os-accounts.sh resolveip.sh JSON.sh rc-bios \
		    ifplug-dhcp-autoconf.sh udhcpc-override.sh udhcpc-hook.sh \
		    ethtool-static-nolink ifupdown-force ssl-create.sh \
		    reset-button.sh check_bios_license logrotate-rsyslog \
		    update-rc.d.sh _bios-script.sh generate_env_user4sql.sh \
		    rcmysql-transient
helper_SCRIPTS	+= ../tests/CI/scriptlib.sh ../tests/CI/weblib.sh \
		    ../tests/CI/restapi-request.sh ../tests/CI/curlbbwget.sh

malamuteexdir	= $(datadir)/@PACKAGE@/examples/config/malamute
malamuteex_DATA	= malamute.cfg

EXTRA_DIST += $(helper_SCRIPTS) $(malamuteex_DATA)

