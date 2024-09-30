# Find PythonQt
#
# Sets PythonQt_FOUND, PYTHONQT_INCLUDE_DIRS, PYTHONQT_LIBRARY, PYTHONQT_LIBRARIES
#

#uncomment to debug
#set(DEBUG_PYTHONQT "1")

# Python is required
find_package(PythonLibs)
if(NOT PYTHONLIBS_FOUND)
	string(ASCII 27 Esc)
	set(ColorReset "${Esc}[m")
	set(ColorBold  "${Esc}[1m")
	message(NOTICE "${ColorBold}Python is required to build PythonQt - it will not be used.${ColorReset}")
else() ####

if(USE_QT6)
	set(PYTHONQT_INSTALL_DIR "/usr/local")
	find_path(PYTHONQT_INCLUDE_DIR PythonQt.h PATHS  "${PYTHONQT_INSTALL_DIR}/include" DOC "Path to the PythonQt include directory" NO_DEFAULT_PATH)
	find_library(PYTHONQT_LIBRARY NAMES PythonQt-Qt6-Python3.10 PATHS "${PYTHONQT_INSTALL_DIR}/lib" DOC "The PythonQt library.")
else()
	find_path(PYTHONQT_INSTALL_DIR NAMES PythonQt.h PythonQt/PythonQt.h include/PythonQt/PythonQt.h include/PythonQt5/PythonQt.h include/Qt5Python38/PythonQt/PythonQt.h include/Qt5Python31/PythonQt/PythonQt.h DOC "Directory where PythonQt was installed.")
	find_path(PYTHONQT_INCLUDE_DIR PythonQt.h PATHS  "${PYTHONQT_INSTALL_DIR}" "${PYTHONQT_INSTALL_DIR}/PythonQt" "${PYTHONQT_INSTALL_DIR}/include/PythonQt" "${PYTHONQT_INSTALL_DIR}/include/PythonQt5" "${PYTHONQT_INSTALL_DIR}/include/Qt5Python38/PythonQt" "${PYTHONQT_INSTALL_DIR}/include/Qt5Python31/PythonQt" DOC "Path to the PythonQt include directory" NO_DEFAULT_PATH)
	find_library(PYTHONQT_LIBRARY NAMES PythonQt QtPython Qt5Python38 Qt5Python31 PythonQt-Qt5-Python3.6 PythonQt-Qt5-Python3.11 PythonQt-Qt5-Python3.12 PATHS "${PYTHONQT_INSTALL_DIR}/lib" DOC "The PythonQt library.")
endif()


mark_as_advanced(PYTHONQT_INSTALL_DIR)
mark_as_advanced(PYTHONQT_INCLUDE_DIR)
mark_as_advanced(PYTHONQT_LIBRARY)

# On linux, also find libutil
if(UNIX AND NOT APPLE)
  find_library(PYTHONQT_LIBUTIL util)
  mark_as_advanced(PYTHONQT_LIBUTIL)
endif()

set(PythonQt_FOUND 0)
if(PYTHONQT_INCLUDE_DIR AND PYTHONQT_LIBRARY)
  # Currently CMake'ified PythonQt only supports building against a python Release build.
  # This applies independently of CTK build type (Release, Debug, ...)
  add_definitions(-DPYTHONQT_USE_RELEASE_PYTHON_FALLBACK)
  set(PythonQt_FOUND 1)
  set(PYTHONQT_INCLUDE_DIRS ${PYTHONQT_INCLUDE_DIR})
  set(PYTHONQT_LIBRARIES ${PYTHONQT_LIBRARY} ${PYTHONQT_LIBUTIL})
endif()

endif() ####

if(DEFINED DEBUG_PYTHONQT)
	message(STATUS "PYTHONQT_INSTALL_DIR ${PYTHONQT_INSTALL_DIR}")
	message(STATUS "PYTHONQT_INCLUDE_DIR ${PYTHONQT_INCLUDE_DIR}")
	message(STATUS "PYTHONQT_LIBRARY ${PYTHONQT_LIBRARY}")
endif()
