#!/bin/bash

cd deps
git clone https://gitlab.com/Dexter9313/octree-file-format.git ;
cd octree-file-format/liboctree ;
git fetch --all
git checkout 1.16.0
mkdir build ; cd build
cmake ..
make package -j
dpkg -i ./*.deb ;
cd ../../.. ;
apt-get install -y libboost-dev ;
cd ..
