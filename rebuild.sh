#!/bin/bash

mkdir -p build
cd build
rm -rf *
cmake ../src
cmake --build .
cmake --build . --target package
