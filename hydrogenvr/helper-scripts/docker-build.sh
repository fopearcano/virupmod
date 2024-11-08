#!/bin/bash

if [ -f ./project_directory.conf ]; then
    . ./project_directory.conf
else
    PROJECT_DIRECTORY="example"
    HVR_DIRECTORY="."
fi
. ./${PROJECT_DIRECTORY}/build.conf

TAG_NAME=$(echo $PROJECT_NAME | tr '[:upper:]' '[:lower:]')_dockerbuild:latest

if [[ "$1" != "--test" ]]
then
	params=$1
elif [[ "$2" != "--test" ]]
then
	params=$2
fi
./docker-makeimage.sh $params

if [[ "$1" == "--test" || "$2" == "--test" ]]
then
	#docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; apt install -y qt5-default libqt5multimedia5-plugins qtdeclarative5-dev-tools ; cd /project ; ./build-linux.sh"
	docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; cd /project ; ./build-linux.sh"
else
	#docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; apt install -y qt5-default libqt5multimedia5-plugins qtdeclarative5-dev-tools ; cd /project ; ./${HVR_DIRECTORY}/ci/gitlab-ci/commons/make-appimage.sh ; chmod -R 777 build/"
	docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; cd /project ; ./${HVR_DIRECTORY}/ci/gitlab-ci/commons/make-appimage.sh ; chmod -R 777 build/"
fi
