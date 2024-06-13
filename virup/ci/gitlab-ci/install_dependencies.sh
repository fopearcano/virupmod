#!/bin/bash

. /etc/os-release

cd deps
git clone https://gitlab.com/Dexter9313/octree-file-format.git ;
cd octree-file-format/liboctree ;
git fetch --all
git checkout 1.16.0
mkdir build ; cd build
cmake ..
if [[ "$ID" == "ubuntu" ]]
then
	make package -j
	dpkg -i ./*.deb ;
else
	make install
fi
cd ../../.. ;
if [[ "$ID" == "ubuntu" ]]
then
	apt-get install -y libboost-dev ;
else
	pacman -S --noconfirm boost
fi
cd ..
