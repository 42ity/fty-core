if sudo docker ps | grep dockerfile/mariadb > /dev/null
then
  echo "dockerfile/mariadb already started"
else
  echo "starting dockerfile/mariadb .."
  sudo docker run -d --name mysql dockerfile/mariadb
  sleep 10
fi
echo "starting eaton/bios .."
container_id=`sudo docker ps -a | grep bios | awk '{print $1}'`
if [ "x$container_id" != "x" ]
then
  sudo docker rm `sudo docker ps -a | grep bios | awk '{print $1}'`
fi
if [[ "x$1" == "x-ti" ]]
then
  sudo docker run -ti --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash
else
  sudo docker run --name bios --link mysql:db -p 8000:8000 eaton/bios /bin/bash -c /usr/local/bin/start_bios.sh
fi
