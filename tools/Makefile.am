# Makefile for tools/ to install some of the scripts for end-user image
EXTRA_DIST =

helperdir	= $(datadir)/@PACKAGE@/scripts
helper_SCRIPTS	= init-os-accounts.sh resolveip.sh JSON.sh ldd-tracer.sh

mysqldir	= $(datadir)/@PACKAGE@/sql/mysql
mysql_DATA	= initdb.sql

mysqlexdir	= $(datadir)/@PACKAGE@/examples/sql/mysql
mysqlex_DATA	= \
		    load_data.sql ci-DC-power-UC1.sql location_topology.sql \
		    power_topology.sql rack_power.sql

malamuteexdir	= $(datadir)/@PACKAGE@/examples/config/malamute
malamuteex_DATA	= malamute.cfg

EXTRA_DIST += $(helper_SCRIPTS) $(mysql_DATA) $(mysqlex_DATA) $(malamuteex_DATA)

