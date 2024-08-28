#!/bin/bash

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -yq build-essential wget cmake git qtbase5-dev libassimp-dev libqt5opengl5-dev qtdeclarative5-dev libqt5svg5-dev qttools5-dev libqt5webkit5-dev qtmultimedia5-dev libqt5xmlpatterns5-dev libpython3-dev libzstd-dev

mkdir deps ; cd deps
# install leap motion sdk
#wget --no-verbose --content-disposition https://warehouse.leapmotion.com/apps/4185/download ;
#tar xzf Leap_Motion_SDK_Linux_*.tgz ;
#cp ./LeapDeveloperKit*/LeapSDK/include/Leap*.h /usr/include ;
#cp ./LeapDeveloperKit*/LeapSDK/lib/x64/libLeap.so /usr/lib ;
# install libktx
git clone --branch v4.3.2 https://github.com/KhronosGroup/KTX-Software.git
mkdir KTX-Software/build ; cd KTX-Software/build
cmake .. -DKTX_FEATURE_STATIC_LIBRARY=ON -DKTX_FEATURE_TESTS=OFF -DKTX_FEATURE_TOOLS=OFF
make install -j $(nproc)
cd ../..
cd ..
# install project additional deps
/project_install_dependencies.sh
