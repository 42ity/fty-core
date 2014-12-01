# Makefile for tools/ to install some of the scripts for end-user image
EXTRA_DIST =

helperdir	= $(datadir)/@PACKAGE@-@VERSION@/scripts
helper_SCRIPTS	= init-os-accounts.sh

mysqldir	= $(datadir)/@PACKAGE@-@VERSION@/sql/mysql
mysql_DATA	= initdb.sql

mysqlexdir	= $(datadir)/@PACKAGE@-@VERSION@/examples/sql/mysql
mysqlex_DATA	= load_data.sql

EXTRA_DIST += $(helper_SCRIPTS) $(mysql_DATA) $(mysqlex_DATA)
