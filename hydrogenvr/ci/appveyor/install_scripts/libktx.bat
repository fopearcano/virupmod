::############################################################################
::# Install libktx 4.2.1
::############################################################################
if NOT EXIST KTX-Software\ (
	git clone https://github.com/KhronosGroup/KTX-Software.git --branch=v4.3.2
	cd KTX-Software
	mkdir build
	cd build
	cmake .. -G %GENERATOR% -DCMAKE_BUILD_TYPE=Release -DKTX_FEATURE_TESTS=OFF -DKTX_FEATURE_TOOLS=OFF
	cmake --build . --config Release
	dir
	dir Release
	cd ..\..
	echo "libktx installed..."
)

set LIBKTX_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/KTX-Software/include
set LIBKTX_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/KTX-Software/build/ktx.lib
set LIBKTX_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\KTX-Software\build\Release\ktx.dll
echo "libktx set..."
