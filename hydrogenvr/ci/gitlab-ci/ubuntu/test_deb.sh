#!/bin/bash

if [[ -f ./project_directory.conf ]]; then . ./project_directory.conf ; else PROJECT_DIRECTORY=example ; HVR_DIRECTORY=. ; fi
. ./${PROJECT_DIRECTORY}/build.conf

cd build
ls
apt-get update
DEBIAN_FRONTEND=noninteractive apt-get -yq install ./*.deb
ldd $(which $PROJECT_NAME)
$PROJECT_NAME --version
