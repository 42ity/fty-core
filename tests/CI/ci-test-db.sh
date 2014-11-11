#/!bin/sh

set -u
set -e
set -x

DB1="core/tools/initdb.sql"
DB2="core/tools/load_data.sql"


mysql -u root < "$DB1"
mysql -u root < "$DB2"
echo "select * from t_bios_asset_element_type;" | mysql -u root box_utf8
