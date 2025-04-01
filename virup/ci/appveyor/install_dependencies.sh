#!/bin/bash

pacman -S --noconfirm --needed mingw-w64-ucrt-x86_64-boost mingw-w64-ucrt-x86_64-qt6-multimedia

cd deps

if [[ ! -d octree-file-format ]]
then
	git clone --branch 1.18.1 https://gitlab.com/Dexter9313/octree-file-format.git
	mkdir octree-file-format/liboctree/build ; cd octree-file-format/liboctree/build
	cmake .. -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=/ucrt64
	cd ../../..
fi

cd octree-file-format/liboctree/build
make install -j $(nproc)
echo "export OCTREE_INCLUDE_DIRS=/ucrt64/include" >> ../../../DEPENDENCIES_ENV
echo "export OCTREE_LIBRARIES=octree" >> ../../../DEPENDENCIES_ENV
cd ../../..
