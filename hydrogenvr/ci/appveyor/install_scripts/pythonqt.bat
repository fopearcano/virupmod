::############################################################################
::# Install PythonQt3.2
::############################################################################
if NOT EXIST pythonqt\ (
	git clone https://github.com/MeVisLab/pythonqt.git --branch=v3.5.4
	cd pythonqt
	qmake
	set CL=/MP
	nmake
	echo "PythonQt installed..."
	cd ..
)
set PYTHONQT_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/src
set PYTHONQT_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/lib/PythonQt-Qt5-Python3.12.lib
set PYTHONQT_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\pythonqt\lib\PythonQt-Qt5-Python3.12.dll
set PYTHONQT_QTALL_INCLUDE_DIRS=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/extensions/PythonQt_QtAll
set PYTHONQT_QTALL_LIBRARIES=%APPVEYOR_BUILD_FOLDER%/deps/pythonqt/lib/PythonQt_QtAll-Qt5-Python3.12.lib
set PYTHONQT_QTALL_SHARED=%APPVEYOR_BUILD_FOLDER%\deps\pythonqt\lib\PythonQt_QtAll-Qt5-Python3.12.dll
echo "PythonQt set..."
