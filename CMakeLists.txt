# Copyright(C) 2018 Florian Cabot < florian.cabot @hotmail.fr >
# 
# This program is free software; you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA 02110 - 1301 USA.

# INIT
cmake_minimum_required(VERSION 3.10.0)

# OPTIONS
option(USE_QT6 "Force using Qt6, else Qt5 will be forced" OFF)

if(USE_QT6)
	set(QT_CMAKE_FILE "cmake/Qt6.cmake")
	message(STATUS "Using Qt6...")
else()
	set(QT_CMAKE_FILE "cmake/Qt5.cmake")
	message(STATUS "Using Qt5...")
endif()

# READ CONFIG FILE AND SET VARIABLES
set(BUILD_CONF "${CMAKE_CURRENT_SOURCE_DIR}/build.conf")
if(NOT EXISTS "${BUILD_CONF}")
	set(BUILD_CONF "${CMAKE_CURRENT_SOURCE_DIR}/build.conf.example")
endif()
message(STATUS "Build configuration file: ${BUILD_CONF}")
file(STRINGS "${BUILD_CONF}" ConfigContents)
foreach(NameAndValue ${ConfigContents})
# Strip leading spaces
  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
# Find variable name
  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
# Find the value
  string(REPLACE "${Name}=" "" Value ${NameAndValue})
# Remove quotes
  string(REPLACE "\"" "" Value ${Value})
# Set the variable
  set(${Name} ${Value})
endforeach()

project(${PROJECT_NAME} CXX)
set(CMAKE_PROJECT_DESCRIPTION ${PROJECT_DESCRIPTION})

# C++ 17
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=gnu++17" COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG("-std=c++1y" COMPILER_SUPPORTS_CXX1Y)
if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17")
elseif(COMPILER_SUPPORTS_CXX1Y)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")
else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support. Please use a different C++ compiler.")
endif()

# For clang - tidy to use(clang - tidy - p)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# WARNINGS
if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wsuggest-override")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	if(DEFINED WERROR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
	endif()
endif()
set(unusedVal "${WERROR}")

# Qt options
if(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

# Get project version from Git
set(PROJECT_VERSION "custom")
if(NOT DEFINED SKIP_GIT_VERSION)
	find_package(Git)
	if(GIT_FOUND)
		execute_process(COMMAND ${GIT_EXECUTABLE} fetch --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --always WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE PROJECT_VERSION)
		string(REGEX REPLACE "\n$" "" PROJECT_VERSION "${PROJECT_VERSION}")
		# prepend 0.0-0-g if no tag to make dpkg happy
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE ret OUTPUT_VARIABLE null ERROR_VARIABLE null)
		if(NOT ret EQUAL "0")
			set(PROJECT_VERSION "0.0-0-g${PROJECT_VERSION}")
		endif()
	endif()
else()
	set(unusedVal "${SKIP_GIT_VERSION}")
endif()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/PROJECT_VERSION "${PROJECT_VERSION}")

# Define project name in source for preprocessor
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DPROJECT_VERSION="${PROJECT_VERSION}")
add_definitions(-DPROJECT_DIRECTORY="${PROJECT_DIRECTORY}")
add_definitions(-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
add_definitions(-DBUILD_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
add_definitions(-DQT_MESSAGELOGCONTEXT)
add_definitions(-DOPENGL_MAJOR_VERSION=${OPENGL_MAJOR_VERSION})
add_definitions(-DOPENGL_MINOR_VERSION=${OPENGL_MINOR_VERSION})
add_definitions(-DOPENGL_PROFILE=${OPENGL_PROFILE})

message(STATUS "Project name: ${PROJECT_NAME}")
message(STATUS "Project version: ${PROJECT_VERSION}")
message(STATUS "Project dir: ${PROJECT_DIRECTORY}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

# Generate .desktop file
set(DESKTOP_FILE_CONTENT "[Desktop Entry]\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Name=${PROJECT_NAME}\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Exec=${PROJECT_NAME}\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Icon=icon\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Type=Application\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Terminal=false\n")
set(DESKTOP_FILE_CONTENT "${DESKTOP_FILE_CONTENT}Categories=Utility;\n")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.desktop "${DESKTOP_FILE_CONTENT}")

# FILES / DIRECTORIES

# Update submodules

execute_process(COMMAND git submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if(EXISTS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty)
	file(GLOB THIRDPARTY_DATA_FILES ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/**/data LIST_DIRECTORIES true)
	file(GLOB ALL_PATHS "${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/*/include")
	set(RESULT "")
	foreach(PATH ${ALL_PATHS})
		if(IS_DIRECTORY ${PATH})
			list(APPEND RESULT ${PATH})
		endif()
	endforeach()
endif()

set(COMMON_INCLUDES ${PROJECT_SOURCE_DIR}/include)
set(THIRDPARTY_INCLUDES ${RESULT})
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/include)
set(TEST_INCLUDES ${PROJECT_SOURCE_DIR}/test ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/test)

file(GLOB_RECURSE ENGINE_HPP_FILES FOLLOW_SYMLINKS ${COMMON_INCLUDES}/*.hpp)
file(GLOB_RECURSE THIRDPARTY_HPP_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/**/include/*.hpp)
file(GLOB_RECURSE PROJECT_HPP_FILES FOLLOW_SYMLINKS ${PROJECT_INCLUDES}/*.hpp)
file(GLOB_RECURSE HPP_FILES FOLLOW_SYMLINKS ${ENGINE_HPP_FILES} ${THIRDPARTY_HPP_FILES} ${PROJECT_HPP_FILES})

file(GLOB_RECURSE ENGINE_SRC_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE THIRDPARTY_SRC_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/**/src/*.cpp)
file(GLOB_RECURSE PROJECT_SRC_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/src/*.cpp)
file(GLOB_RECURSE SRC_FILES FOLLOW_SYMLINKS ${ENGINE_SRC_FILES} ${THIRDPARTY_SRC_FILES} ${PROJECT_SRC_FILES})
file(GLOB_RECURSE MAIN_FILE FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/src/main.cpp)
list(REMOVE_ITEM SRC_FILES ${MAIN_FILE})

file(GLOB_RECURSE ENGINE_TEST_HPP_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/test/*.hpp)
file(GLOB_RECURSE TEST_HPP_FILES FOLLOW_SYMLINKS ${ENGINE_TEST_HPP_FILES} ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/test/*.hpp)
file(GLOB_RECURSE ENGINE_TEST_CPP_FILES FOLLOW_SYMLINKS ${PROJECT_SOURCE_DIR}/test/*.cpp)
file(GLOB_RECURSE TEST_SRC_FILES FOLLOW_SYMLINKS ${ENGINE_TEST_CPP_FILES} ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/test/*.cpp)

FOREACH(PATH ${THIRDPARTY_DATA_FILES})
	get_filename_component(FOO "${PATH}" DIRECTORY)
	get_filename_component(LIB_NAME "${FOO}" NAME)
	set(LIBRARIES_DIRS "${LIBRARIES_DIRS} ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/${LIB_NAME}/include ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/${LIB_NAME}/src")
ENDFOREACH(PATH)

# DEPENDENCIES
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
set(CONAN_CMAKE_SILENT_OUTPUT TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake/modules ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/cmake/modules)

if(DOC_ONLY)
	include(cmake/Doc.cmake)
	return()
endif()
if(CLANG_TOOLS_ONLY)
	include(cmake/ClangDevTools.cmake)
	return()
endif()

if(HYDROGENVR_CONAN_INSTALLED)
	list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")
	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup(NO_OUTPUT_DIRS)
	
	message(STATUS "Adding conan installed libraries to the search path")
endif()

macro(hydrogenvr_find_packages)
	set(options FORCE_REQUIRED)
	cmake_parse_arguments(FN "${options}" "" "" ${ARGN})
if(USE_QT6)
	set(REQUIRED_LIBS
    #   Cmake Pkg Prefix   Version     Conan Pkg         Required
        "assimp            4.1.0       assimp/5.0.1      true"
        "OpenVR            1.12.5      openvr/1.12.5     true"
		"LeapMotion        0.0.0       N/A               false"
		"PythonQt          0.0.0       N/A               false"
		"PythonQt_QtAll    0.0.0       N/A               false"
		"libktx            0.0.0       N/A               false"
		"ZSTD              0.0.0       N/A               false"
		"Qt6Gamepad        0.0.0       N/A               false"
    )
else()
	set(REQUIRED_LIBS
    #   Cmake Pkg Prefix   Version     Conan Pkg         Required
        "assimp            4.1.0       assimp/5.0.1      true"
        "OpenVR            1.12.5      openvr/1.12.5     true"
		"LeapMotion        0.0.0       N/A               false"
		"PythonQt          0.0.0       N/A               false"
		"PythonQt_QtAll    0.0.0       N/A               false"
		"libktx            0.0.0       N/A               false"
		"ZSTD              0.0.0       N/A               false"
    )
endif()

	foreach(PACKAGE ${REQUIRED_LIBS})
        string(REGEX REPLACE "[ \t\r\n]+" ";" PACKAGE_SPLIT ${PACKAGE})
        list(GET PACKAGE_SPLIT 0 PACKAGE_PREFIX)
        list(GET PACKAGE_SPLIT 1 PACKAGE_VERSION)
        list(GET PACKAGE_SPLIT 2 PACKAGE_CONAN)
		list(GET PACKAGE_SPLIT 3 PACKAGE_REQUIRED)

		string(TOUPPER ${PACKAGE_PREFIX} PACKAGE_PREFIX_UPPER)
		string(TOLOWER ${PACKAGE_PREFIX} PACKAGE_PREFIX_LOWER)
		set(${PACKAGE_PREFIX_UPPER}_INCLUDE_DIRS $ENV{${PACKAGE_PREFIX_UPPER}_INCLUDE_DIRS})
		set(${PACKAGE_PREFIX_UPPER}_LIBRARIES $ENV{${PACKAGE_PREFIX_UPPER}_LIBRARIES})
		if((DEFINED ${PACKAGE_PREFIX_UPPER}_INCLUDE_DIRS) AND (DEFINED ${PACKAGE_PREFIX_UPPER}_LIBRARIES))
			# If includes and libraries dirs are already provided, no more work is necessary 
			set(${PACKAGE_PREFIX}_FOUND true)
		else()
			# This function is called twice, once to check if the packages exist on the system already
			# and a second time to check if conan installed them properly. The second check passes in FORCE_REQUIRED
			if (FN_FORCE_REQUIRED OR PACKAGE_REQUIRED)
				find_package(${PACKAGE_PREFIX} REQUIRED)
			else()
				find_package(${PACKAGE_PREFIX})
			endif()
			
			if ((NOT ${PACKAGE_PREFIX}_FOUND) AND (NOT (${PACKAGE_CONAN} STREQUAL "N/A")) AND PACKAGE_REQUIRED)
				list(APPEND CONAN_REQUIRED_LIBS ${PACKAGE_CONAN})
			else()
				# For each package found by conan a target is configured that avoids to manually set include_dirs 
				# and stuff. For comptability reasons we still set a legacy findPackage.cmake style PACKAGE_LIBRARIES 
				# variable and INCLUDE_DIRS variable
				if(TARGET ${PACKAGE_PREFIX_LOWER}::${PACKAGE_PREFIX_LOWER})
					if(NOT DEFINED ${PACKAGE_PREFIX_UPPER}_LIBRARIES)
						set(${PACKAGE_PREFIX_UPPER}_LIBRARIES "${PACKAGE_PREFIX_LOWER}::${PACKAGE_PREFIX_LOWER}")
					endif()
					if(NOT DEFINED ${PACKAGE_PREFIX_UPPER}_INCLUDE_DIRS)
						get_target_property(${PACKAGE_PREFIX_UPPER}_INCLUDE_DIRS ${PACKAGE_PREFIX_LOWER}::${PACKAGE_PREFIX_LOWER} INTERFACE_INCLUDE_DIRECTORIES)
					endif()
				endif()
			endif()
		endif()
    endforeach()
    unset(FN_FORCE_REQUIRED)
endmacro()

# Attempt to locate any packages that are required and report the missing ones in CONAN_REQUIRED_LIBS
hydrogenvr_find_packages()

FOREACH(PATH ${THIRDPARTY_DATA_FILES})
	get_filename_component(FOO "${PATH}" DIRECTORY)
	get_filename_component(LIB_NAME "${FOO}" NAME)
	set(LIBRARIES_DIRS "${LIBRARIES_DIRS} ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/${LIB_NAME}/include ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/${LIB_NAME}/src")
ENDFOREACH(PATH)

# Determine OpenVR version
set(OPENVR_HEADER "${OPENVR_INCLUDE_DIRS}/openvr/openvr.h")
# Read the lines containing version information
file(STRINGS ${OPENVR_HEADER} OPENVR_VERSION_LINES
     REGEX "k_nSteamVRVersion(Major|Minor|Build)")
# Initialize version components
set(OPENVR_VERSION_MAJOR "0")
set(OPENVR_VERSION_MINOR "0")
set(OPENVR_VERSION_BUILD "0")
# Extract the version components
foreach(version_line ${OPENVR_VERSION_LINES})
    if(version_line MATCHES "k_nSteamVRVersionMajor = ([0-9]+);")
        set(OPENVR_VERSION_MAJOR "${CMAKE_MATCH_1}")
    elseif(version_line MATCHES "k_nSteamVRVersionMinor = ([0-9]+);")
        set(OPENVR_VERSION_MINOR "${CMAKE_MATCH_1}")
    elseif(version_line MATCHES "k_nSteamVRVersionBuild = ([0-9]+);")
        set(OPENVR_VERSION_BUILD "${CMAKE_MATCH_1}")
    endif()
endforeach()

# Compose the full version string
set(OPENVR_VERSION "${OPENVR_VERSION_MAJOR}.${OPENVR_VERSION_MINOR}.${OPENVR_VERSION_BUILD}")
message(STATUS "OpenVR Version: ${OPENVR_VERSION}")

# Pass OpenVR version to compiler
add_definitions(-DOPENVR_VERSION_MAJOR=${OPENVR_VERSION_MAJOR})
add_definitions(-DOPENVR_VERSION_MINOR=${OPENVR_VERSION_MINOR})
add_definitions(-DOPENVR_VERSION_BUILD=${OPENVR_VERSION_BUILD})

#Qt
include(${QT_CMAKE_FILE})

# Install any missing dependencies with conan install
if (CONAN_REQUIRED_LIBS)
	message(STATUS "Packages ${CONAN_REQUIRED_LIBS} not found!")
	# Use Conan to fetch the libraries that aren't found
	# Download conan.cmake automatically, you can also just copy the conan.cmake file
	if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
		message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
		file(DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
						"${CMAKE_BINARY_DIR}/conan.cmake")
	endif()
	include(${CMAKE_BINARY_DIR}/conan.cmake)

	set(CONAN_LIB_OPTIONS
		qt:with_sqlite3=False
		qt:openssl=False
		qt:with_harfbuzz=False
		qt:with_freetype=False
		qt:with_pcre2=False
    )

	conan_add_remote(NAME bincrafters
						URL https://api.bintray.com/conan/bincrafters/public-conan)
	conan_cmake_run(REQUIRES ${CONAN_REQUIRED_LIBS}
					OPTIONS ${CONAN_LIB_OPTIONS}
					BUILD missing
					GENERATORS cmake cmake_find_package_multi
	)

	include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

	list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")
	list(APPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}")
	conan_basic_setup(NO_OUTPUT_DIRS)

	set(HYDROGENVR_CONAN_INSTALLED TRUE CACHE BOOL "If true, the following builds will add conan to the lib search path" FORCE)

	# Now that we've installed what we are missing, try to locate them again
	hydrogenvr_find_packages(FORCE_REQUIRED)
	include(${QT_CMAKE_FILE})
endif()

# LeapMotion
if(LeapMotion_FOUND)
	message(STATUS "LeapMotion found")
	add_definitions(-DLEAP_MOTION)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${LEAPMOTION_INCLUDE_DIRS})
	set(LD_LIBS ${LD_LIBS} ${LEAPMOTION_LIBRARIES})
else()
	message(STATUS "LeapMotion not found")
endif()

# Python
if((PythonQt_FOUND) OR (PythonQt_QtAll_FOUND))
	if(PythonQt_FOUND)
		message(STATUS "PythonQt found")
		add_definitions(-DPYTHONQT)
		set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${PYTHONQT_INCLUDE_DIRS})
		set(LD_LIBS ${LD_LIBS} ${PYTHONQT_LIBRARIES})
	else()
		message(STATUS "PythonQt NOT found")
	endif()
	if(PythonQt_QtAll_FOUND)
		message(STATUS "PythonQt_QtAll found")
		add_definitions(-DPYTHONQT_QTALL)
		set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${PYTHONQT_QTALL_INCLUDE_DIRS})
		set(LD_LIBS ${LD_LIBS} ${PYTHONQT_QTALL_LIBRARIES})
	else()
		message(STATUS "PythonQt_QtAll NOT found")
	endif()
	find_package(PythonLibs)
	find_package(Python COMPONENTS Interpreter Development)
	message(STATUS "PythonQt : using Python 3")
	message(STATUS "PYTHONPATH : ${Python_STDARCH}")
	add_definitions(-DPYTHON_VERSION="${Python_VERSION}")
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${PYTHON_INCLUDE_DIRS})
	set(LD_LIBS ${LD_LIBS} ${PYTHON_LIBRARIES})
else()
	message(STATUS "PythonQt not found")
endif()

# libktx
if(libktx_FOUND)
	message(STATUS "libktx found")
	add_definitions(-DLIBKTX)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${LIBKTX_INCLUDE_DIRS})
	set(LD_LIBS ${LD_LIBS} ${LIBKTX_LIBRARIES})
else()
	message(STATUS "libktx not found")
endif()

# libzstd
if(ZSTD_FOUND)
	message(STATUS "libzstd found")
	add_definitions(-DLIBZSTD)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${ZSTD_INCLUDE_DIRS})
	set(LD_LIBS ${LD_LIBS} ${ZSTD_LIBRARIES})
else()
	message(STATUS "libzstd not found")
endif()

#Additional
if(EXISTS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/cmake/Dependencies.cmake)
	include(${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/cmake/Dependencies.cmake)
endif()
if(libktx_FOUND)
	add_definitions(-DLIBKTX)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${LIBKTX_INCLUDE_DIRS})
endif()
if(ZSTD_FOUND)
	add_definitions(-DLIBZSTD)
	set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${ZSTD_INCLUDE_DIRS})
endif()

# INCLUDE
set(EXTERNAL_LIBS_INCLUDE_DIRS ${EXTERNAL_LIBS_INCLUDE_DIRS} ${ASSIMP_INCLUDE_DIRS} ${OPENVR_INCLUDE_DIRS} ${PROJECT_INCLUDE_DIRS})
include_directories(${COMMON_INCLUDES} ${THIRDPARTY_INCLUDES} ${PROJECT_INCLUDES} ${TEST_INCLUDES} SYSTEM ${EXTERNAL_LIBS_INCLUDE_DIRS})

# LINKING SETUP
set(LD_LIBS ${LD_LIBS} ${ASSIMP_LIBRARIES} ${OPENVR_LIBRARIES} ${PROJECT_LIBRARIES})

# Find .so in working directory
if(NOT WIN32)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath='$ORIGIN:/usr/lib/${PROJECT_NAME}'")
endif()

# Prevent console from popping on Windows
IF((DEFINED WIN32) AND ($ENV{HIDE_CONSOLE}))
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /entry:mainCRTStartup")
ENDIF()

# RUN MOC (defined in Qt specific cmake file)
run_moc()

# COMMON OBJECTS
add_library(objects OBJECT ${SRC_FILES} ${MOC_FILES})
target_link_libraries(objects ${LD_LIBS})
foreach(EXT_PROJ IN LISTS PROJECT_EXT_PROJECTS)
	add_dependencies(objects ${EXT_PROJ})
endforeach()

# TESTS
add_executable(tests ${TEST_SRC_FILES} ${TEST_MOC_FILES} $<TARGET_OBJECTS:objects>)
target_link_libraries(tests ${LD_LIBS})

# EXECUTABLE : PROJECT_NAME
add_executable(${PROJECT_NAME} ${MAIN_FILE} $<TARGET_OBJECTS:objects>)
target_link_libraries(${PROJECT_NAME} ${LD_LIBS})


#SETUP DATA DIRECTORY

# core
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data ${PROJECT_BINARY_DIR}/data
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# project
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/data ${PROJECT_BINARY_DIR}/data/${PROJECT_DIRECTORY}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# thirdparty

FOREACH(PATH ${THIRDPARTY_DATA_FILES})
	get_filename_component(FOO "${PATH}" DIRECTORY)
	get_filename_component(LIB_NAME "${FOO}" NAME)
	execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/thirdparty/${LIB_NAME}/data ${PROJECT_BINARY_DIR}/data/${LIB_NAME}
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
ENDFOREACH(PATH)

# translations

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/translations ${PROJECT_BINARY_DIR}/data/translations
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


# INSTALL RULES / NOT FOR WINDOWS
if(NOT WIN32)
	# get distro by sourcing /etc/os-release
	file(STRINGS "/etc/os-release" ConfigContents)
	foreach(NameAndValue ${ConfigContents})
	#Strip leading spaces
	  string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
	#Find variable name
	  string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
	#Find the value
	  string(REPLACE "${Name}=" "" Value ${NameAndValue})
	#Remove quotes
	  string(REPLACE "\"" "" Value ${Value})
	#Set the variable
	  set("OSRELEASE_${Name}" ${Value})
	endforeach()

	set(DEB_DEP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_DIRECTORY}/ci/gitlab-ci/${OSRELEASE_ID}/${OSRELEASE_VERSION_ID}/DEPENDENCIES)
	if(NOT EXISTS ${DEB_DEP_FILE})
		set(DEB_DEP_FILE ${CMAKE_CURRENT_SOURCE_DIR}/ci/gitlab-ci/${OSRELEASE_ID}/${OSRELEASE_VERSION_ID}/DEPENDENCIES)
	endif()
	message(STATUS "deb DEPENDENCIES file : ${DEB_DEP_FILE}")

	INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME} DESTINATION bin)
	# Ubuntu 20.04 has a package
	if((NOT "${OSRELEASE_ID}" STREQUAL "ubuntu") OR (NOT "${OSRELEASE_VERSION_ID}" STREQUAL "20.04"))
		INSTALL(FILES ${OPENVR_LIBRARIES} DESTINATION lib/${PROJECT_NAME})
	endif()
	if(LeapMotion_FOUND)
		INSTALL(FILES ${LEAPMOTION_LIBRARIES} DESTINATION lib/${PROJECT_NAME})
	endif()
	# Ubuntu 18.04 has a package
	if(PythonQt_FOUND AND ((NOT ${OSRELEASE_ID} STREQUAL "ubuntu") OR (NOT ${OSRELEASE_VERSION_ID} STREQUAL "18.04")))
		INSTALL(FILES ${PYTHONQT_LIBRARIES} DESTINATION lib/${PROJECT_NAME})
	endif()
	if(PythonQt_QtAll_FOUND AND ((NOT ${OSRELEASE_ID} STREQUAL "ubuntu") OR (NOT ${OSRELEASE_VERSION_ID} STREQUAL "18.04")))
		INSTALL(FILES ${PYTHONQT_QTALL_LIBRARIES} DESTINATION lib/${PROJECT_NAME})
	endif()
	INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/data DESTINATION share/${PROJECT_NAME})
	INSTALL(DIRECTORY ${Python_STDARCH}/ DESTINATION share/${PROJECT_NAME}/python)
	if(EXISTS ${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/cmake/Install.cmake)
		include(${PROJECT_SOURCE_DIR}/${PROJECT_DIRECTORY}/cmake/Install.cmake)
	endif()

	SET(CPACK_GENERATOR "DEB")
	SET(CPACK_PACKAGE_NAME ${PROJECT_NAME})
	SET(CPACK_DEBIAN_PACKAGE_MAINTAINER ${PROJECT_MAINTAINER})
	SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
	SET(CPACK_SYSTEM_NAME "${OSRELEASE_NAME}-${OSRELEASE_VERSION_ID}")
	if(EXISTS ${DEB_DEP_FILE})
		file(READ ${DEB_DEP_FILE} CPACK_DEBIAN_PACKAGE_DEPENDS)
		string(REGEX REPLACE "\n$" "" CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}")
	endif()

	INCLUDE(CPack)

	# Create uninstall target
	add_custom_target(uninstall
			COMMAND xargs rm < install_manifest.txt && rm install_manifest.txt
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Uninstalling executable..."
			VERBATIM)
endif()

# Copy Qt DLL files for windows
if(MSVC)
	copy_Qt_deps(${PROJECT_NAME})
endif()

# Including extra cmake rules
include(cmake/ClangDevTools.cmake)
include(cmake/Doc.cmake)
include(cmake/GLSLValidator.cmake)

# Translation
if(PROJECT_TRANSLATE)
	set(TRANSLATION_FILES ${TRANSLATION_FILES} ${PROJECT_HPP_FILES} ${PROJECT_SRC_FILES})
endif()
if(PROJECT_TRANSLATE_ENGINE)
	set(TRANSLATION_FILES ${TRANSLATION_FILES} ${ENGINE_HPP_FILES} ${ENGINE_SRC_FILES})
endif()
if(PROJECT_TRANSLATE_THIRDPARTY)
	set(TRANSLATION_FILES ${TRANSLATION_FILES} ${THIRDPARTY_HPP_FILES} ${THIRDPARTY_SRC_FILES})
endif()

add_custom_target(translation-update
	COMMAND lupdate ${TRANSLATION_FILES} -ts ${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_fr.ts
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating translation (.ts) file."
	VERBATIM)
add_custom_target(translation-release
	COMMAND lrelease ${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_fr.ts
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating translation (.qm) file."
	VERBATIM)
