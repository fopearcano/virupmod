::############################################################################
::# Install ASSIMP 4.1
::############################################################################
if NOT EXIST assimp\ (
	git clone https://github.com/assimp/assimp.git --branch=v4.1.0
	cd assimp
	mkdir build
	cd build
	cmake .. -G %GENERATOR%
	cmake --build . --config Release
	cd ..\..
	echo "Assimp installed..."
)

set ASSIMP_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/assimp/include;%APPVEYOR_BUILD_FOLDER%/deps/assimp/build/include
set ASSIMP_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/assimp/build/code/Release/assimp-vc140-mt.lib
set ASSIMP_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\assimp\build\code\Release\assimp-vc140-mt.dll

echo "Assimp set..."
