mkdir %RELEASE_DIR_NAME%
cd %RELEASE_DIR_NAME%
mkdir platforms
mkdir imageformats
mkdir audio
mkdir mediaservice
mkdir gamepads
::# for local dev ; building pythonqt is always a pain, we can do it only once on AppVeyor
mkdir dev
mkdir dev\pythonqt
robocopy %PYTHONQT_INCLUDE_DIRS% dev\pythonqt\src /e
robocopy %APPVEYOR_BUILD_FOLDER%\deps\pythonqt\lib dev\pythonqt\lib /e
mkdir dev\pythonqt_qtall
robocopy %PYTHONQT_QTALL_INCLUDE_DIRS% dev\pythonqt_qtall\src /e
robocopy %APPVEYOR_BUILD_FOLDER%\deps\pythonqt\lib dev\pythonqt_qtall\lib /e
::# end for local dev
cd ..
copy %PROJECT_NAME%.exe %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Core.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Gui.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Widgets.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Svg.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5OpenGL.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5PrintSupport.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5MultimediaWidgets.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5QuickWidgets.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Multimedia.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Quick.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Sql.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Test.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5XmlPatterns.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Qml.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5QmlModels.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Network.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Xml.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Gamepad.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5WebEngineCore.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5WebEngineWidgets.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5WebChannel.dll %RELEASE_DIR%
copy %QTPATH%\bin\Qt5Positioning.dll %RELEASE_DIR%
copy %QTPATH%\plugins\platforms\qwindows.dll %RELEASE_DIR%\platforms
copy %QTPATH%\plugins\imageformats\*.dll %RELEASE_DIR%\imageformats
copy %QTPATH%\plugins\audio\*.dll %RELEASE_DIR%\audio
copy %QTPATH%\plugins\mediaservice\*.dll %RELEASE_DIR%\mediaservice
copy %QTPATH%\plugins\gamepads\*.dll %RELEASE_DIR%\gamepads
copy %OPENVR_SHARED% %RELEASE_DIR%
copy %ASSIMP_SHARED% %RELEASE_DIR%
copy %LIBKTX_SHARED% %RELEASE_DIR%
::# - copy %LEAPMOTION_SHARED% %RELEASE_DIR%
copy %PYTHON_PATH%\python312.dll %RELEASE_DIR%
copy %PYTHONQT_SHARED% %RELEASE_DIR%
copy %PYTHONQT_QTALL_SHARED% %RELEASE_DIR%
appveyor DownloadFile %PYTHON_URL% -FileName python.zip
7z x python.zip -o.\python > nul
del python.zip
7z x python\python312.zip -o%RELEASE_DIR%\python > nul
move python\*.pyd %RELEASE_DIR%\python
move data %RELEASE_DIR%
set USER_BEFORE_ARCHIVING=..\..\%PROJECT_DIRECTORY%\ci\appveyor\before_archiving.bat
IF EXIST %USER_BEFORE_ARCHIVING% (%USER_BEFORE_ARCHIVING%)
7z a -tzip %RELEASE_DIR_NAME%.zip %RELEASE_DIR%
