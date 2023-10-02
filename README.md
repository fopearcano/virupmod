[![Build status](https://ci.appveyor.com/api/projects/status/f1djey7fu7ibh31u/branch/master?svg=true)](https://ci.appveyor.com/project/Dexter9313/virup/branch/master)

# VIRUP

The VIrtual Reality Universe Project uses astrophysical data to propose a virtual reality visualization of it.

## Documentation

Quick start instructions can be found here : [Quick start instructions](https://www.epfl.ch/labs/lastro/public-outreach/virup/documentation/).
Code documentation can be found here : [Code documentation](https://dexter9313.gitlab.io/virup/).

## Releases

You can download releases for GNU/Linux and Windows produced by the engine build system through Gitlab CI and Appveyor on Gitlab : [VIRUP Releases](https://gitlab.com/Dexter9313/virup/-/releases).

## Installation and requirements

### Microsoft Windows

Simply run the VIRUP-VERSION-windows-ARCH_setup.exe setup wizard, where VERSION is the desired version and ARCH your system architecture.

If you want to run VIRUP from a portable zip archive instead, you will need to manually install this (if you don't have it on your system already) :
* [Microsoft Visual C++ 2015](https://www.microsoft.com/en-US/download/details.aspx?id=48145) (zip installation only; it is shipped with the setup wizard)

### GNU/Linux binaries

You can then install the DEB package you want from the Releases page or use the portable zip version.

### GNU/Linux building from source

To install the requirements on Ubuntu, see the following scripts :
* *ci/gitlab-ci/ubuntu/VERSION/install_dependencies.sh*
* *ci/gitlab-ci/commons/install_dependencies.sh*
* *virup/ci/gitlab-ci/install_dependencies.sh*

For other distributions, it shouldn't be too hard to adapt the scripts.

In summary, your environment should have installed :
* A C++14 compiler (g++ for example)
* CMake >= 3.10
* [liboctree](https://gitlab.com/Dexter9313/octree-file-format/blob/master/liboctree/)
* OpenGL dev
* Qt 5 Core and Gui (qtbase5-dev packages on Ubuntu)
* [OpenVR](https://github.com/ValveSoftware/openvr)
* (Optional) [Leap Motion SDK 2.3.1](https://developer.leapmotion.com/sdk/v2)
* (Optional) [PythonQt](https://mevislab.github.io/pythonqt/) and its dependencies.
* (Optional) Qt 5 Gamepad


Then clone this repository. We now suppose the root directory of the repository is stored in the $VIRUP_ROOT_DIR variable.

        cd $VIRUP_ROOT_DIR
        ./build-linux.sh $(nproc)
        cd build
        sudo make install

Optionally, you can generate a deb package to make installation managing easier if you are on a debian-based system. The package name will be "virup".

        cd $VIRUP_ROOT_DIR/build/
        cmake ..
        make package -j $(nproc)
        sudo dpkg -i ./build/*.deb

In both examples, you can replace $(nproc) with any number of CPU threads.

## Uninstall

If the deb method for installation was used :

        sudo apt-get autoremove virup

If the make install method for installation was used, uninstallation can only be done if the build directory has been left untouched since installation (at least the install_manifest.txt file within it) :

        cd $VIRUP_ROOT_DIR
        cd build
        sudo make uninstall

As of now, this method can leave some empty directories in your file system though... Check install_manifest.txt content yourself if you want to clean everything properly.
