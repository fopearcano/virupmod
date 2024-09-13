#!/bin/bash

cd deps
git clone https://gitlab.com/Dexter9313/octree-file-format.git ;
cd octree-file-format/liboctree ;
git fetch --all
git checkout 1.17.2
mkdir build ; cd build
cmake ..
make -j$(nproc) install
cd ../../.. ;
pacman -S --noconfirm boost
cd ..
