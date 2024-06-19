#!/bin/bash

if [ ! -f "./build.conf" ]; then
	. ./build.conf.example
else
	. ./build.conf
fi

mkdir build ; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWERROR=true
export VERSION=$(cat PROJECT_VERSION)
make -j $(nproc)
make package
./$PROJECT_NAME --version
./tests

