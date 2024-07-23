#!/bin/bash

. ./project_directory.conf || PROJECT_DIRECTORY=hydrogenvr/example
. ./${PROJECT_DIRECTORY}/build.conf

./build-linux.sh
if [ $? -ne 0 ]; then
	exit
fi
cd build
./$PROJECT_NAME
cd ..
