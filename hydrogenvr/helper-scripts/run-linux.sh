#!/bin/bash

if [ -f ./project_directory.conf ]; then
    . ./project_directory.conf
else
    PROJECT_DIRECTORY="example"
    HVR_DIRECTORY="."
fi
. ./${PROJECT_DIRECTORY}/build.conf

./build-linux.sh
if [ $? -ne 0 ]; then
	exit
fi
cd build
./$PROJECT_NAME
cd ..
