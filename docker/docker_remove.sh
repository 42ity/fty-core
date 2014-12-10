case $1 in
 all)
   echo "removing bios container .."
   sudo docker rm `sudo docker ps -a | grep bios | awk '{print $1}'`
   echo "removing mysql container .."
   sudo docker rm `sudo docker ps -a | grep mysql | awk '{print $1}'`;;
  *)
   echo "removing $1 container .."
   sudo docker rm `sudo docker ps -a | grep $1 | awk '{print $1}'`;;
esac
