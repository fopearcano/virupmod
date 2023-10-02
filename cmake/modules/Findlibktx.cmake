# Find libktx
#
# Sets libktx_FOUND, LIBKTX_INCLUDE_DIRS, LIBKTX_LIBRARIES
#

# Threads
find_package(Threads)

# Python2
find_path(LIBKTX_INSTALL_DIR NAMES include/ktx.h DOC "Directory where libktx was installed.")
find_path(LIBKTX_INCLUDE_DIRS ktx.h PATHS "${LIBKTX_INSTALL_DIR}/include/ktx.h" DOC "Path to the libktx include directory")
find_library(LIBKTX_LIBRARIES NAMES ktx PATHS "${LIBKTX_INSTALL_DIR}/lib" DOC "The libktx library.")

set(libktx_FOUND 0)
if(LIBKTX_INCLUDE_DIRS AND LIBKTX_LIBRARIES)
  set(libktx_FOUND 1)
  set(LIBKTX_LIBRARIES ${LIBKTX_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_LD_LIBS} dl)
endif()

message(STATUS "KTX : ${libktx_FOUND} | ${LIBKTX_INSTALL_DIR} | ${LIBKTX_INCLUDE_DIRS} | ${LIBKTX_LIBRARIES}")
