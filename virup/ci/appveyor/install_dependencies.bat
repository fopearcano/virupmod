::############################################################################
::# Install liboctree
::############################################################################
mkdir octree
cd octree
set URL="https://gitlab.com/api/v4/projects/9621748/packages/generic/releases/1.17.3/liboctree-1.17.3-windows-%BUILD_TYPE%.zip"
appveyor DownloadFile %URL% -FileName octree.zip
7z x octree.zip > nul
move liboctree* liboctree
set OCTREE_INCLUDE_DIR=%APPVEYOR_BUILD_FOLDER%/deps/octree/
set OCTREE_LIBRARY=%APPVEYOR_BUILD_FOLDER%/deps/octree/liboctree/octree.lib
set OCTREE_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\octree\liboctree\octree.dll
cd ..

set BOOST_ROOT=C:/Libraries/boost_1_83_0
