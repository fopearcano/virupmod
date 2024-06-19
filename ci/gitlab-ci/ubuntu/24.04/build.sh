#!/bin/bash

./ci/gitlab-ci/commons/main_build.sh
cd build
make validate-glsl
