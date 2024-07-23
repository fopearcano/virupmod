::############################################################################
::# Install OpenVR
::############################################################################
IF NOT EXIST openvr\ (
	mkdir openvr
	cd openvr
	mkdir openvr
	cd openvr
	appveyor DownloadFile https://raw.githubusercontent.com/ValveSoftware/openvr/v1.12.5/headers/openvr.h -FileName openvr.h
	cd ..
	IF "%BUILD_TYPE%" == "64bit" (
		appveyor DownloadFile https://raw.githubusercontent.com/ValveSoftware/openvr/v1.12.5/lib/win64/openvr_api.lib -FileName openvr_api.lib
		appveyor DownloadFile https://raw.githubusercontent.com/ValveSoftware/openvr/v1.12.5/bin/win64/openvr_api.dll -FileName openvr_api.dll
	) ELSE (
		appveyor DownloadFile https://raw.githubusercontent.com/ValveSoftware/openvr/v1.12.5/lib/win32/openvr_api.lib -FileName openvr_api.lib
		appveyor DownloadFile https://raw.githubusercontent.com/ValveSoftware/openvr/v1.12.5/bin/win32/openvr_api.dll -FileName openvr_api.dll
	)
	echo "OpenVR installed..."
	cd ..
)
set OPENVR_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/openvr/
set OPENVR_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/openvr/openvr_api.lib
set OPENVR_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\openvr\openvr_api.dll
echo "OpenVR set..."
