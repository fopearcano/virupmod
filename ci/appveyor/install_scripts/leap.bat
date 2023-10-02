::############################################################################
::# Install Leap Motion SDK
::############################################################################
IF NOT EXIST leap\ (
	mkdir leap
	cd leap
	appveyor DownloadFile https://warehouse.leapmotion.com/apps/4183/download -FileName leap.zip
	7z x leap.zip > nul
	move LeapDeveloperKit* Leap
	echo "LeapMotionSDK installed..."
	cd ..
)
IF "%BUILD_TYPE%" == "64bit" (set ARCH=x64) ELSE (set ARCH=x86)
set LEAPMOTION_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/leap/Leap/LeapSDK/include
set LEAPMOTION_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/leap/Leap/LeapSDK/lib/%ARCH%/Leap.lib
set LEAPMOTION_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\leap\Leap\LeapSDK\lib\%ARCH%\Leap.dll

echo "LeapMotionSDK set..."
