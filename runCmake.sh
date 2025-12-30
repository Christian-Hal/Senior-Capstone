#!/bin/bash

rm -r build
mkdir build
cd build
cmake ..
cmake --build .
cd Debug
./Capstone.exe