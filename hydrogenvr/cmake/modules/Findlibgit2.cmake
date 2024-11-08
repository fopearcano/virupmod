# Find libgit2
#
# Sets libgit2_FOUND, libgit2_INCLUDE_DIRS, libgit2_LIBRARIES
#

# pkg-config
find_package(PkgConfig)
if(PkgConfig_FOUND)
	pkg_check_modules(libgit2 libgit2)
endif()
