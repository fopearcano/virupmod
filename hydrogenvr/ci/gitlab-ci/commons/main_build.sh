#!/bin/bash

. ./project_directory.conf
. ./${PROJECT_DIRECTORY}/build.conf

mkdir build ; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DWERROR=true
export VERSION=$(cat PROJECT_VERSION)
make -j $(nproc)
make package
./$PROJECT_NAME --version
./tests

