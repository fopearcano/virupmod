FROM ubuntu:24.04
ARG PROJECT_INSTALL_DEPS
ARG HVR_DIRECTORY
COPY ./project_directory.conf* /project_directory.conf
COPY ./$HVR_DIRECTORY/ci/gitlab-ci/ubuntu/24.04/install_dependencies.sh /install_dependencies.sh
COPY ./$HVR_DIRECTORY/ci/gitlab-ci/ubuntu/install_dependencies.sh /$HVR_DIRECTORY/ci/gitlab-ci/ubuntu/install_dependencies.sh
COPY $PROJECT_INSTALL_DEPS /project_install_dependencies.sh
RUN ./install_dependencies.sh
