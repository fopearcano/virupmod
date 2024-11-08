#!/bin/bash

if [[ -f ./project_directory.conf ]]; then . ./project_directory.conf ; else PROJECT_DIRECTORY=example ; HVR_DIRECTORY=. ; fi
./$HVR_DIRECTORY/ci/gitlab-ci/commons/main_build.sh
