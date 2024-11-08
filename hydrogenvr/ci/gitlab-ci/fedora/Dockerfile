FROM fedora:40
ARG PROJECT_INSTALL_DEPS
ARG HVR_DIRECTORY
COPY ./project_directory.conf* /project_directory.conf
COPY ./$HVR_DIRECTORY/ci/gitlab-ci/fedora/install_dependencies.sh /install_dependencies.sh
COPY $PROJECT_INSTALL_DEPS /project_install_dependencies.sh
RUN ./install_dependencies.sh
