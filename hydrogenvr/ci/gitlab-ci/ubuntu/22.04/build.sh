#!/bin/bash

./hydrogenvr/ci/gitlab-ci/commons/main_build.sh
cd build
make validate-glsl
