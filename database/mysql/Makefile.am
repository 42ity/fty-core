# Makefile for database/mysql/ to install some of the files for end-user image
EXTRA_DIST =

mysqldir	= $(datadir)/@PACKAGE@/sql/mysql
mysql_DATA	= initdb.sql

mysqlexdir	= $(datadir)/@PACKAGE@/examples/sql/mysql
mysqlex_DATA	= \
		    load_data.sql ci-DC-power-UC1.sql location_topology.sql \
		    power_topology.sql rack_power.sql load_ROZLAB.sql \
		    load_data_test_restapi.sql

EXTRA_DIST += $(mysql_DATA) $(mysqlex_DATA)

