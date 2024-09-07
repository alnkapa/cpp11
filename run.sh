#!/bin/bash

# mkdir -p build
#cd build
# rm -rf *
#cmake .. -DCMAKE_INSTALL_PREFIX=~
#cmake --build .
# sudo apt-get install libssl-dev

seq 0 9 | nc localhost 1234&
seq 10 19 | nc localhost 1234&
wait
