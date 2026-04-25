pkill -9 run_datanode
pkill -9 run_proxy

./project/cmake/build/run_datanode 127.0.0.1:17600 & 
./project/cmake/build/run_datanode 127.0.0.1:17601 & 
./project/cmake/build/run_datanode 127.0.0.1:17602 & 
./project/cmake/build/run_datanode 127.0.0.1:17603 & 
./project/cmake/build/run_datanode 127.0.0.1:17604 & 
./project/cmake/build/run_datanode 127.0.0.1:17605 & 
./project/cmake/build/run_datanode 127.0.0.1:17606 & 
./project/cmake/build/run_datanode 127.0.0.1:17607 & 
./project/cmake/build/run_datanode 127.0.0.1:17608 & 
./project/cmake/build/run_datanode 127.0.0.1:17609 & 
./project/cmake/build/run_datanode 127.0.0.1:17610 & 
./project/cmake/build/run_datanode 127.0.0.1:17611 & 
./project/cmake/build/run_datanode 127.0.0.1:17612 & 
./project/cmake/build/run_datanode 127.0.0.1:17613 & 
./project/cmake/build/run_datanode 127.0.0.1:17614 & 
./project/cmake/build/run_datanode 127.0.0.1:17615 & 
./project/cmake/build/run_datanode 127.0.0.1:17616 & 
./project/cmake/build/run_datanode 127.0.0.1:17617 & 
./project/cmake/build/run_datanode 127.0.0.1:17618 & 
./project/cmake/build/run_datanode 127.0.0.1:17619 & 
./project/cmake/build/run_datanode 127.0.0.1:17620 & 
./project/cmake/build/run_datanode 127.0.0.1:17621 & 
./project/cmake/build/run_datanode 127.0.0.1:17622 & 
./project/cmake/build/run_datanode 127.0.0.1:17623 & 
./project/cmake/build/run_datanode 127.0.0.1:17624 & 
./project/cmake/build/run_datanode 127.0.0.1:17625 & 
./project/cmake/build/run_datanode 127.0.0.1:17626 & 
./project/cmake/build/run_datanode 127.0.0.1:17627 & 
./project/cmake/build/run_datanode 127.0.0.1:17628 & 
./project/cmake/build/run_datanode 127.0.0.1:17629 & 
./project/cmake/build/run_datanode 127.0.0.1:17630 & 
./project/cmake/build/run_datanode 127.0.0.1:17631 & 
./project/cmake/build/run_datanode 127.0.0.1:17632 & 
./project/cmake/build/run_datanode 127.0.0.1:17633 & 
./project/cmake/build/run_datanode 127.0.0.1:17634 & 
./project/cmake/build/run_datanode 127.0.0.1:17635 & 
./project/cmake/build/run_datanode 127.0.0.1:17636 & 
./project/cmake/build/run_datanode 127.0.0.1:17637 & 
./project/cmake/build/run_datanode 127.0.0.1:17638 & 
./project/cmake/build/run_datanode 127.0.0.1:17639 & 
./project/cmake/build/run_datanode 127.0.0.1:17640 & 
./project/cmake/build/run_datanode 127.0.0.1:17641 & 

sleep 5s

./project/cmake/build/run_proxy 127.0.0.1:50405 &
./project/cmake/build/run_proxy 127.0.0.1:50406 &
./project/cmake/build/run_proxy 127.0.0.1:50407 &
./project/cmake/build/run_proxy 127.0.0.1:50408 &
./project/cmake/build/run_proxy 127.0.0.1:50409 &
./project/cmake/build/run_proxy 127.0.0.1:50410 &

