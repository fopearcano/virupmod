#!/bin/bash

set -e

. ./project_directory.conf
. ./${PROJECT_DIRECTORY}/build.conf

rm -rf build || true
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make -j $(nproc)
make install DESTDIR=AppDir

# now, build AppImage using linuxdeploy and linuxdeploy-plugin-qt
# download linuxdeploy and its Qt plugin

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

# make them executable and extract them (to make them compatible with docker
chmod +x linuxdeploy*.AppImage
./linuxdeploy-plugin-qt-x86_64.AppImage --appimage-extract
./linuxdeploy-x86_64.AppImage --appimage-extract
rm linuxdeploy*.AppImage # important ! else the AppImage of the Qt plugin will be used (and will fail on docker)

# initialize AppDir, bundle shared libraries for QtQuickApp, use Qt plugin to bundle additional resources, and build AppImage, all in one single command
VERSION=$(cat PROJECT_VERSION) ./squashfs-root/AppRun --appdir AppDir --plugin qt --output appimage -d ${PROJECT_NAME}.desktop -i ../$PROJECT_ICON
