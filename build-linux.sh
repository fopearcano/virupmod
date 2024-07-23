#!/bin/bash -e

. ./project_directory.conf || PROJECT_DIRECTORY=hydrogenvr/example
. ./${PROJECT_DIRECTORY}/build.conf

mkdir -p build
cd build
if [[ -v CMAKE_BUILD_TYPE ]];
then
	cmake .. -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE
else
	cmake .. -DCMAKE_BUILD_TYPE=Release
fi

if [[ $# -ne 1 ]]
then
	ncpus=$(lscpu --parse -a | grep -v "^#" | wc -l)
else
	ncpus="$@"
fi

echo "Check GLSL code"
make validate-glsl
echo "make -j $ncpus"
make -j $ncpus
