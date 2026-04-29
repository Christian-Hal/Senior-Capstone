#!/bin/bash

cd build
cmake --build .
cd Debug
./MockUp.exe