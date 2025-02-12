pkill -9 run_datanode
pkill -9 run_proxy

./project/cmake/build/run_datanode 0.0.0.0:17600 & 
./project/cmake/build/run_datanode 0.0.0.0:17601 & 
./project/cmake/build/run_datanode 0.0.0.0:17602 & 
./project/cmake/build/run_datanode 0.0.0.0:17603 & 

sleep 5s

./project/cmake/build/run_proxy 0.0.0.0:50405  & 

