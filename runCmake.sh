#!/bin/bash

cd build
cmake --build .
cd Debug
./Capstone.exe