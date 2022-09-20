#/bin/bash

./ci/gitlab-ci/commons/install_dependencies.sh
cd deps;
git clone https://github.com/Orochimarufan/PythonQt ;
cd PythonQt ;
git checkout 18d4c249ca9b0003cfb10ad711c60fb7f9d5f79b ;
# Python >= 10
sed -i "s/pydebug.h/cpython\/pydebug.h/" ./src/PythonQt.cpp
sed -i "s/#undef _POSIX_THREADS//" ./src/PythonQtPythonInclude.h
cd build ;
cmake .. -DBUILD_SHARED_LIBS=ON -DPythonQt_Python3=ON;
make -j 8 ;
make install ;
cd ../../..
# install openvr
apt-get install -y libopenvr-dev libqt5gamepad5-dev
rm -rf /deps
