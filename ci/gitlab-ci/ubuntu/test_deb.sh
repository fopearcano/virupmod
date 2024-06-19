#!/bin/bash

if [ ! -f "./build.conf" ]; then
	. ./build.conf.example
else
	. ./build.conf
fi
cd build
ls
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -yq install ./*.deb
ldd $(which $PROJECT_NAME)
$PROJECT_NAME --version
