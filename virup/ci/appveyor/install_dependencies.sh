#!/bin/bash

pacman -S mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-qt5-multimedia

cd deps

if [[ ! -d octree ]]
then
	mkdir octree
	cd octree
	curl "https://gitlab.com/api/v4/projects/9621748/packages/generic/releases/1.17.3/liboctree-1.17.3-windows-${BUILD_TYPE}.zip" > octree.zip
	unzip octree.zip
	mv liboctree* liboctree
	cd ..
fi

cd octree/liboctree
cp -r liboctree /ucrt64/include
cp octree.dll /ucrt64/bin/liboctree.dll
cp octree.lib /ucrt64/lib/liboctree.lib
echo "export OCTREE_INCLUDE_DIRS=/ucrt64/include" >> ../../../DEPENDENCIES_ENV
echo "export OCTREE_LIBRARIES=octree" >> ../../../DEPENDENCIES_ENV
cd ../..
