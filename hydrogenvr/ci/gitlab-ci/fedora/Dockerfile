FROM fedora:40
ARG PROJECT_INSTALL_DEPS
COPY ./hydrogenvr/ci/gitlab-ci/fedora/install_dependencies.sh /install_dependencies.sh
COPY $PROJECT_INSTALL_DEPS /project_install_dependencies.sh
RUN ./install_dependencies.sh
