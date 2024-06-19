::############################################################################
::# Install PythonQt3.2
::############################################################################
if NOT EXIST pythonqt\ (
	mkdir pythonqt
	cd pythonqt
	appveyor DownloadFile https://sourceforge.net/projects/pythonqt/files/pythonqt/PythonQt-3.2/PythonQt3.2.zip/download -FileName pythonqt.zip
	7z x pythonqt.zip > nul
	cd PythonQt3.2
	del PythonQt.pro
	del build\python.prf
	del build\PythonQt.prf
	del build\PythonQt_QtAll.prf
	move %APPVEYOR_BUILD_FOLDER%\misc\PythonQt.pro .
	move %APPVEYOR_BUILD_FOLDER%\misc\python.prf .\build\python.prf
	move %APPVEYOR_BUILD_FOLDER%\misc\PythonQt.prf .\build\PythonQt.prf
	move %APPVEYOR_BUILD_FOLDER%\misc\PythonQt_QtAll.prf .\build\PythonQt_QtAll.prf
	IF "%BUILD_TYPE%" == "64bit" (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
	) ELSE (
		call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"
	)
	qmake
	nmake
	echo "PythonQt installed..."
	cd ..\..
)
set PYTHONQT_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/PythonQt3.2/src
set PYTHONQT_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/PythonQt3.2/lib/PythonQt-Qt5-Python373.lib
set PYTHONQT_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\pythonqt\PythonQt3.2\lib\PythonQt-Qt5-Python373.dll
set PYTHONQT_QTALL_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/PythonQt3.2/extensions/PythonQt_QtAll
set PYTHONQT_QTALL_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/PythonQt3.2/lib/PythonQt_QtAll-Qt5-Python373.lib
set PYTHONQT_QTALL_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\pythonqt\PythonQt3.2\lib\PythonQt_QtAll-Qt5-Python373.dll
echo "PythonQt set..."
