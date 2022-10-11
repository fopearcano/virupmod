#!/bin/bash

if [ ! -f "./build.conf" ]; then
	. ./build.conf.example
else
	. ./build.conf
fi

TAG_NAME=$(echo $PROJECT_NAME | tr '[:upper:]' '[:lower:]')_dockerbuild:latest

./docker-makeimage.sh

if [[ "$1" == "test" ]]
then
	docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; apt install -y qt5-default libqt5multimedia5-plugins qtdeclarative5-dev-tools ; cd /project ; ./build-linux.sh"
else
	docker run --rm -v $(pwd):/project $TAG_NAME /bin/bash -c "git config --global --add safe.directory /project ; apt install -y qt5-default libqt5multimedia5-plugins qtdeclarative5-dev-tools ; cd /project ; ./ci/gitlab-ci/commons/make-appimage.sh ; chmod -R 777 build/"
fi
