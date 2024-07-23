# Find PythonQt_QtAll
#
# Sets PythonQt_QtAll_FOUND, PYTHONQT_QTALL_INCLUDE_DIRS, PYTHONQT_QTALL_LIBRARY, PYTHONQT_QTALL_LIBRARIES
#

if(USE_QT6)
	set(PYTHONQT_QTALL_INSTALL_DIR "/usr/local")
	find_path(PYTHONQT_QTALL_INCLUDE_DIR PythonQt_QtAll.h PATHS  "${PYTHONQT_QTALL_INSTALL_DIR}/include" DOC "Path to the PythonQt_QtAll include directory" NO_DEFAULT_PATH)
	find_library(PYTHONQT_QTALL_LIBRARY NAMES PythonQt_QtAll-Qt6-Python3.10 PATHS "${PYTHONQT_QTALL_INSTALL_DIR}/lib" DOC "The PythonQt_QtAll library.")
else()
	find_path(PYTHONQT_QTALL_INSTALL_DIR NAMES PythonQt/PythonQt_QtAll.h include/PythonQt/PythonQt_QtAll.h include/PythonQt/extensions/PythonQt_QtAll include/Qt5Python38/PythonQt/PythonQt_QtAll.h include/Qt5Python31/PythonQt/PythonQt_QtAll.h include/PythonQt5/PythonQt_QtAll.h DOC "Directory where PythonQt_QtAll.h was installed.")
	find_path(PYTHONQT_QTALL_INCLUDE_DIR PythonQt_QtAll.h PATHS "${PYTHONQT_QTALL_INSTALL_DIR}/PythonQt" "${PYTHONQT_QTALL_INSTALL_DIR}/include/PythonQt" "${PYTHONQT_QTALL_INSTALL_DIR}/include/PythonQt/extensions/PythonQt_QtAll" "${PYTHONQT_QTALL_INSTALL_DIR}/include/PythonQt5" "${PYTHONQT_QTALL_INSTALL_DIR}/include/Qt5Python38/PythonQt" "${PYTHONQT_QTALL_INSTALL_DIR}/include/Qt5Python31/PythonQt" "${PYTHONQT_QTALL_INSTALL_DIR}/PythonQt/extensions/PythonQt_QtAll/" DOC "Path to the PythonQt_QtAll include directory" NO_DEFAULT_PATH)
	find_library(PYTHONQT_QTALL_LIBRARY NAMES PythonQt_QtAll QtPython_QtAll Qt5Python38_QtAll Qt5Python31_QtAll PythonQt_QtAll-Qt5-Python3.6 PythonQt_QtAll-Qt5-Python3.11 PythonQt_QtAll-Qt5-Python3.12 PATHS "${PYTHONQT_QTALL_INSTALL_DIR}/extensions/PythonQt_QtAll" DOC "The PythonQt_QtAll library.")
endif()

mark_as_advanced(PYTHONQT_QTALL_INSTALL_DIR)
mark_as_advanced(PYTHONQT_QTALL_INCLUDE_DIR)
mark_as_advanced(PYTHONQT_QTALL_LIBRARY)

set(PythonQt_QtAll_FOUND 0)
if(PYTHONQT_QTALL_INCLUDE_DIR AND PYTHONQT_QTALL_LIBRARY)
  # Currently CMake'ified PYTHONQT_QTALL only supports building against a python Release build.
  # This applies independently of CTK build type (Release, Debug, ...)
  add_definitions(-DPYTHONQT_QTALL_USE_RELEASE_PYTHON_FALLBACK)
  set(PythonQt_QtAll_FOUND 1)
  set(PYTHONQT_QTALL_INCLUDE_DIRS ${PYTHONQT_QTALL_INCLUDE_DIR})
  set(PYTHONQT_QTALL_LIBRARIES ${PYTHONQT_QTALL_LIBRARY} ${PYTHONQT_QTALL_LIBUTIL})
endif()

