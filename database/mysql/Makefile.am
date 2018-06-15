# Makefile for database/mysql/ to install some of the files for end-user image
EXTRA_DIST =

mysqldir	= $(datadir)/@PACKAGE@/sql/mysql
mysql_DATA	= initdb.sql \
			0001_device_type_extension.sql \
			0002_v_web_element_view.sql \
			0003_device_type_extension.sql \
			0004_ext_cascade.sql \
			0005_super_parent_names.sql \
			0006_super_parent_names.sql \
			0007_device_type_extension.sql \
			0008_ext_names.sql \
			0009_device_type_extension.sql \
			0010_super_parent_names.sql \
			0011_default_rc.sql \
			0012_device_type_extension.sql \
			0013_user_management.sql \
			0014_user_preferences.sql

mysqlexdir	= $(datadir)/@PACKAGE@/examples/sql/mysql
mysqlex_DATA	= \
		    load_data.sql ci-DC-power-UC1.sql location_topology.sql \
		    power_topology.sql rack_power.sql load_ROZLAB.sql \
		    load_data_test_restapi.sql load_data.csv

EXTRA_DIST += $(mysql_DATA) $(mysqlex_DATA)
