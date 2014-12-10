export build_dir=../build
export temp_dir=./temp 

echo "preparing $temp_dir .. "
mkdir -p $temp_dir
cp ressources/* $temp_dir
cp $build_dir/simple $temp_dir
cp $build_dir/netmon $temp_dir
cp $build_dir/driver-nmap $temp_dir
cp $build_dir/.libs/bios_web.so.0.0.0 $temp_dir
cp $build_dir/tntnet.xml.example $temp_dir/tntnet.xml
sed -ri 's:/usr/local/share/core-0.1/web:/var/www:' $temp_dir/tntnet.xml
sed -ri 's:/usr/local/lib/core:/usr/local/lib:' $temp_dir/tntnet.xml

cp ../tools/initdb.sql $temp_dir 
cp ../tools/load_data.sql $temp_dir

echo "building docker eaton/bios .."
sudo docker build -t eaton/bios $temp_dir

echo "cleaning .."
sudo docker rmi $(sudo docker images -q -f dangling=true)
rm $temp_dir/*
rm -d $temp_dir

echo "done"
sudo docker images | grep eaton/bios

echo "exporting eaton/bios into eaton-bios.tar .."
sudo docker save eaton/bios > eaton-bios.tar
ls -lh  eaton-bios.tar
