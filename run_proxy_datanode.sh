pkill -9 run_datanode
pkill -9 run_proxy

./project/cmake/build/run_datanode 0.0.0.0:17600 & 
./project/cmake/build/run_datanode 0.0.0.0:17601 & 
./project/cmake/build/run_datanode 0.0.0.0:17602 & 
./project/cmake/build/run_datanode 0.0.0.0:17603 & 
./project/cmake/build/run_datanode 0.0.0.0:17604 & 
./project/cmake/build/run_datanode 0.0.0.0:17605 & 
./project/cmake/build/run_datanode 0.0.0.0:17606 & 
./project/cmake/build/run_datanode 0.0.0.0:17607 & 
./project/cmake/build/run_datanode 0.0.0.0:17608 & 
./project/cmake/build/run_datanode 0.0.0.0:17609 & 
./project/cmake/build/run_datanode 0.0.0.0:17610 & 
./project/cmake/build/run_datanode 0.0.0.0:17611 & 

sleep 5s

./project/cmake/build/run_proxy 0.0.0.0:50405  & 

