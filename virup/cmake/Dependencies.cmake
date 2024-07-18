set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/virup/cmake/modules)

set(OCTREE_INCLUDE_DIR $ENV{OCTREE_INCLUDE_DIR})
set(OCTREE_LIBRARY $ENV{OCTREE_LIBRARY})

if((NOT DEFINED OCTREE_INCLUDE_DIR) OR (NOT DEFINED OCTREE_LIBRARY))
       find_package(Octree REQUIRED)
endif()

find_package (Threads)

cmake_policy(SET CMP0057 NEW)
find_package(Boost REQUIRED)
set(PROJECT_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
set(PROJECT_LIBRARIES ${Boost_LIBRARIES})

if(USE_QT6)
	find_package(Qt6 COMPONENTS Multimedia REQUIRED)
	set(PROJECT_INCLUDE_DIRS ${Qt6Multimedia_INCLUDE_DIRS})
	set(PROJECT_LIBRARIES Qt6::Multimedia)
else()
	find_package(Qt5 COMPONENTS Multimedia REQUIRED)
	set(PROJECT_INCLUDE_DIRS ${Qt5Multimedia_INCLUDE_DIRS})
	set(PROJECT_LIBRARIES Qt5::Multimedia)
endif()

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${OCTREE_INCLUDE_DIR})
set(PROJECT_LIBRARIES ${PROJECT_LIBRARIES} ${OCTREE_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

# For float128
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	message(STATUS "Using g++ float128 for quadruple precision")
	add_definitions(-DUSE_FLOAT128)
	set(PROJECT_LIBRARIES ${PROJECT_LIBRARIES} -lquadmath)
else()
	message(STATUS "Using boost cpp_dec_float_50 for quadruple precision")
endif()
