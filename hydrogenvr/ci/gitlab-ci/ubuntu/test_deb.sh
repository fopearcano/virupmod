#!/bin/bash

. ./project_directory.conf
. ./${PROJECT_DIRECTORY}/build.conf

cd build
ls
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -yq install ./*.deb
ldd $(which $PROJECT_NAME)
$PROJECT_NAME --version
