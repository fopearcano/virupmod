FROM ubuntu:22.04
ARG PROJECT_INSTALL_DEPS
COPY ./hydrogenvr/ci/gitlab-ci/ubuntu/22.04/install_dependencies.sh /install_dependencies.sh
COPY ./hydrogenvr/ci/gitlab-ci/ubuntu/install_dependencies.sh /hydrogenvr/ci/gitlab-ci/ubuntu/install_dependencies.sh
COPY $PROJECT_INSTALL_DEPS /project_install_dependencies.sh
RUN ./install_dependencies.sh
