::############################################################################
::# Install ASSIMP 4.1
::############################################################################
if NOT EXIST assimp\ (
	git clone https://github.com/assimp/assimp.git --branch=v5.4.2
	cd assimp
	mkdir build
	cd build
	cmake .. -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_TESTS=OFF
	cmake --build . --config Release
	cd ..\..
	echo "Assimp installed..."
)

set ASSIMP_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/assimp/include;%APPVEYOR_BUILD_FOLDER%/deps/assimp/build/include
set ASSIMP_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/assimp/build/lib/assimp-vc142-mt.lib
set ASSIMP_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\assimp\build\bin\assimp-vc142-mt.dll

echo "Assimp set..."
