# SPDX-FileCopyrightText: Copyright 2024 Narr the Reg
# SPDX-License-Identifier: GPL-3.0-or-later

# Enable modules to include each other's files
include_directories(.)

# Set compilation flags
if (MSVC)
    set(CMAKE_CONFIGURATION_TYPES Debug Release CACHE STRING "" FORCE)

    # Avoid windows.h junk
    add_definitions(-DNOMINMAX)

    # Avoid windows.h from including some usually unused libs like winsocks.h, since this might cause some redefinition errors.
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    # Ensure that projects are built with Unicode support.
    add_definitions(-DUNICODE -D_UNICODE)

    # /W4                 - Level 4 warnings
    # /MP                 - Multi-threaded compilation
    # /Zi                 - Output debugging information
    # /Zm                 - Specifies the precompiled header memory allocation limit
    # /Zo                 - Enhanced debug info for optimized builds
    # /permissive-        - Enables stricter C++ standards conformance checks
    # /EHsc               - C++-only exception handling semantics
    # /utf-8              - Set source and execution character sets to UTF-8
    # /volatile:iso       - Use strict standards-compliant volatile semantics.
    # /Zc:externConstexpr - Allow extern constexpr variables to have external linkage, like the standard mandates
    # /Zc:inline          - Let codegen omit inline functions in object files
    # /Zc:preprocessor    - Enable standards-conforming preprocessor
    # /Zc:throwingNew     - Let codegen assume `operator new` (without std::nothrow) will never return null
    # /GT                 - Supports fiber safety for data allocated using static thread-local storage
    add_compile_options(
        /MP
        /Zm200
        /Zo
        /permissive-
        /EHsc
        /std:c++20
        /utf-8
        /volatile:iso
        /Zc:externConstexpr
        /Zc:inline
        /Zc:preprocessor
        /Zc:throwingNew
        /GT

        # Modules
        /experimental:module- # Explicitly disable module support due to conflicts with precompiled headers.

        # External headers diagnostics
        /external:anglebrackets # Treats all headers included by #include <header>, where the header file is enclosed in angle brackets (< >), as external headers
        /external:W0            # Sets the default warning level to 0 for external headers, effectively disabling warnings for them.

        # Warnings
        /W4
        /WX

        /we4800 # Implicit conversion from 'type' to bool. Possible information loss
        /we4062 # Enumerator 'identifier' in a switch of enum 'enumeration' is not handled
        /we4189 # 'identifier': local variable is initialized but not referenced
        /we4242 # 'identifier': conversion from 'type1' to 'type2', possible loss of data
        /we4244 # 'conversion': conversion from 'type1' to 'type2', possible loss of data
        /we4245 # 'conversion': conversion from 'type1' to 'type2', signed/unsigned mismatch
        /we4254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
        /we4265 # 'class': class has virtual functions, but destructor is not virtual
        /we4388 # 'expression': signed/unsigned mismatch
        /we4389 # 'operator': signed/unsigned mismatch
        /we4456 # Declaration of 'identifier' hides previous local declaration
        /we4457 # Declaration of 'identifier' hides function parameter
        /we4458 # Declaration of 'identifier' hides class member
        /we4459 # Declaration of 'identifier' hides global declaration
        /we4505 # 'function': unreferenced local function has been removed
        /we4547 # 'operator': operator before comma has no effect; expected operator with side-effect
        /we4549 # 'operator1': operator before comma has no effect; did you intend 'operator2'?
        /we4555 # Expression has no effect; expected expression with side-effect
        /we4826 # Conversion from 'type1' to 'type2' is sign-extended. This may cause unexpected runtime behavior.
        /we5038 # data member 'member1' will be initialized after data member 'member2'
        /we5233 # explicit lambda capture 'identifier' is not used
        /we5245 # 'function': unreferenced function with internal linkage has been removed

        /wd4100 # 'identifier': unreferenced formal parameter
        /wd4324 # 'struct_name': structure was padded due to __declspec(align())
        /wd4201 # nonstandard extension used : nameless struct/union
        /wd4702 # unreachable code (when used with LTO)
    )

    set(CMAKE_EXE_LINKER_FLAGS_DEBUG   "/DEBUG /MANIFEST:NO" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/DEBUG /MANIFEST:NO /INCREMENTAL:NO /OPT:REF,ICF" CACHE STRING "" FORCE)
else()
    add_compile_options(
        -fwrapv

        -Werror=all
        -Werror=extra
        -Werror=missing-declarations
        -Werror=shadow
        -Werror=unused

        -Wno-attributes
        -Wno-invalid-offsetof
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -Werror=conversion

        -Wno-sign-conversion
        -Wno-cast-function-type

        $<$<CXX_COMPILER_ID:Clang>:-fsized-deallocation>
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES Clang) # Clang or AppleClang
        add_compile_options(
            -Wno-braced-scalar-init
            -Wno-unused-private-field
            -Wno-nullability-completeness
            -Werror=shadow-uncaptured-local
            -Werror=implicit-fallthrough
            -Werror=type-limits
        )
    endif()

    if (ARCHITECTURE_x86_64)
        add_compile_options("-mcx16")
    endif()

    if (APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL Clang)
        add_compile_options("-stdlib=libc++")
    endif()

    # GCC bugs
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "11" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # These diagnostics would be great if they worked, but are just completely broken
        # and produce bogus errors on external libraries like fmt.
        add_compile_options(
            -Wno-array-bounds
            -Wno-stringop-overread
            -Wno-stringop-overflow
        )
    endif()

    # Set file offset size to 64 bits.
    #
    # On modern Unixes, this is typically already the case. The lone exception is
    # glibc, which may default to 32 bits. glibc allows this to be configured
    # by setting _FILE_OFFSET_BITS.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR MINGW)
        # GNU ar: Create thin archive files.
        # Requires binutils-2.19 or later.
        set(CMAKE_C_ARCHIVE_CREATE   "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_C_ARCHIVE_APPEND   "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> qcTP <TARGET> <LINK_FLAGS> <OBJECTS>")
        set(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> qTP  <TARGET> <LINK_FLAGS> <OBJECTS>")
    endif()
endif()

add_subdirectory(common)
add_subdirectory(the_dude_to_human)
