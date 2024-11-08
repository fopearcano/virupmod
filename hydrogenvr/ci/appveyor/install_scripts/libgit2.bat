::############################################################################
::# Install libgit2
::############################################################################
IF NOT EXIST libgit2\ (
	git clone https://github.com/libgit2/libgit2.git --branch=v1.7.2

	cd libgit2
	mkdir build
	cd build
	cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
	cmake --build . --config Release
	dir

	cd ..\..
	echo "libgit2 installed..."
)
set libgit2_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/libgit2/include/
set libgit2_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/libgit2/build/git2.lib
set libgit2_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\libgit2\build\git2.dll

echo "ZSTD set..."
