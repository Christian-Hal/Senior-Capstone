#!/bin/bash

cd build
cmake --build . --config Release
cd Release
MockUp.exe &
