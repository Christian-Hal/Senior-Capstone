#!/bin/bash

cd build
cmake --build .
cd Debug
./Mockup.exe
