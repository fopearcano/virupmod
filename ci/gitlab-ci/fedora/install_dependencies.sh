#!/usr/bin/bash

dnf -y install cmake make automake gcc gcc-c++ kernel-devel assimp-devel qt5-qtbase-devel git
git clone --depth 1 --branch v1.12.5 https://github.com/ValveSoftware/openvr.git
cd openvr/ ; mkdir build ; cd build
cmake ..
make install
cd ../.. ; rm -rf openvr/
