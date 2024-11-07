# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: GPL-3.0-or-later

# Disable tests in all externals supporting the standard option name
set(BUILD_TESTING OFF)

# Build only static externals
set(BUILD_SHARED_LIBS OFF)

# Skip install rules for all externals
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL ON)

# Sqlite
add_subdirectory(sqlite)
target_include_directories(sqlite PUBLIC ./sqlite)

# getopt
if (MSVC)
    add_subdirectory(getopt)
endif()

# Libssh2
add_subdirectory(libssh2)
