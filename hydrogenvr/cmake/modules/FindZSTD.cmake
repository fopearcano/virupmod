# taken from: https://github.com/facebook/folly/blob/master/CMake/FindZstd.cmake
# SPDX-FileCopyrightText: Facebook, Inc. and its affiliates.
# SPDX-License-Identifier: Apache-2.0
#
# - Try to find Facebook zstd library
# This will define
# ZSTD_FOUND
# ZSTD_INCLUDE_DIRS
# ZSTD_LIBRARIES
#

find_path(ZSTD_INCLUDE_DIRS NAMES zstd.h)
find_library(ZSTD_LIBRARIES NAMES zstd)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    ZSTD DEFAULT_MSG
    ZSTD_LIBRARIES ZSTD_INCLUDE_DIRS
)

mark_as_advanced(ZSTD_INCLUDE_DIRS ZSTD_LIBRARIES)
