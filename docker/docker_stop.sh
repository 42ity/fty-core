case $1 in
 all)
   echo "stopping bios container .."
   sudo docker stop `sudo docker ps -a | grep bios | awk '{print $1}'`
   echo "stopping mysql container .."
   sudo docker stop `sudo docker ps -a | grep mysql | awk '{print $1}'`
   sudo docker ps -a ;;
  *)
  echo "stopping $1 container .."
   sudo docker stop `sudo docker ps -a | grep $1 | awk '{print $1}'`
   sudo docker ps -a | grep $1;;

esac

