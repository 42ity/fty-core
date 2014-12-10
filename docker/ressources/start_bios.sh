echo  "initializing db .."
mysql < /tmp/initdb.sql
mysql < /tmp/load_data.sql
mysql < /tmp/patch.sql

echo "Starting SASL auth .."
service saslauthd start

echo "Starting web srv .."
tntnet -c /usr/local/etc/tntnet.xml &

echo "starting nut .."
service nut-server start

echo "starting simple .."
cd /usr/local/bin
./simple 

