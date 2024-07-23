#!/bin/bash

. ./project_directory.conf || PROJECT_DIRECTORY=hydrogenvr/example
. ./${PROJECT_DIRECTORY}/build.conf

IMAGE_NAME=$(echo $PROJECT_NAME | tr '[:upper:]' '[:lower:]')_dockerbuild
TAG_NAME=${IMAGE_NAME}:latest

if [[ "$(docker images | grep $IMAGE_NAME | wc -l)" != "0" ]]
then
	exit 0
fi

dockerfile=./hydrogenvr/ci/gitlab-ci/ubuntu/20.04/Dockerfile
if [[ -n "$1" ]]
then
	dockerfile=./hydrogenvr/ci/gitlab-ci/$1/Dockerfile
fi

echo "Building from Dockerfile:$dockerfile"

PROJ_INST_DEP=./$PROJECT_DIRECTORY/ci/gitlab-ci/install_dependencies.sh
if [ ! -f $PROJ_INST_DEP ]; then PROJ_INST_DEP=./hydrogenvr/example/ci/gitlab-ci/install_dependencies.sh; fi
docker build --pull --build-arg PROJECT_INSTALL_DEPS=$PROJ_INST_DEP -t ${TAG_NAME} -f $dockerfile --no-cache --pull .

