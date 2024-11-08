::############################################################################
::# Install ZSTD
::############################################################################
IF NOT EXIST zstd\ (
	git clone https://github.com/facebook/zstd.git --branch=v1.5.6
	copy %APPVEYOR_BUILD_FOLDER%\%HVR_DIRECTORY%\misc\libzstd-dll.rc zstd\build\VS2010\libzstd-dll\libzstd-dll.rc

	cd zstd\build\cmake
	mkdir build
	cd build
	cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
	cmake --build . --config Release

	cd ..\..\..\..
	echo "ZSTD installed..."
)
set ZSTD_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/zstd/lib/
set ZSTD_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/zstd/build/cmake/build/lib/Release/zstd.lib
::# set ZSTD_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\zstd\build\cmake\build\lib\zstd.dll
echo "ZSTD set..."
