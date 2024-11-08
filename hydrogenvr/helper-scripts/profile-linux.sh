#!/bin/bash

#to start profiling :
#callgrind_control -i on
#to stop profiling :
#callgrind_control -i off

if [ -f ./project_directory.conf ]; then
    . ./project_directory.conf
else
    PROJECT_DIRECTORY="example"
    HVR_DIRECTORY="."
fi
. ./${PROJECT_DIRECTORY}/build.conf

export CMAKE_BUILD_TYPE=Debug
./build-linux.sh
if [ $? -ne 0 ]; then
	exit
fi
cd build
valgrind --tool=callgrind --instr-atstart=no ./$PROJECT_NAME
kcachegrind callgrind.*
rm callgrind.*
cd ..
