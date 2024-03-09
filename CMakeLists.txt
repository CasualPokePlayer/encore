# CMake 3.12 required for 20 to be a valid value for CXX_STANDARD
cmake_minimum_required(VERSION 3.15)

# Don't override the warning flags in MSVC:
cmake_policy(SET CMP0092 NEW)
# Enforce new LTO setting
cmake_policy(SET CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
# Honor visibility properties for all targets
# Set the default so subdirectory cmake_minimum_required calls won't unset the policy.
cmake_policy(SET CMP0063 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0063 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/externals/cmake-modules")
include(DownloadExternals)
include(CMakeDependentOption)

project(encore LANGUAGES C CXX ASM)

# Some submodules like to pick their own default build type if not specified.
# Make sure we default to Release build type always, unless the generator has custom types.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

if (APPLE)
    # Silence warnings on empty objects, for example when platform-specific code is #ifdef'd out.
    set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_CXX_ARCHIVE_CREATE "<CMAKE_AR> Scr <TARGET> <LINK_FLAGS> <OBJECTS>")
    set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")
    set(CMAKE_CXX_ARCHIVE_FINISH "<CMAKE_RANLIB> -no_warning_for_no_symbols -c <TARGET>")

    if (IOS)
        # Minimum iOS 14
        set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")

        # Enable searching CMAKE_PREFIX_PATH for bundled dependencies.
        set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
        set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
    else()
        # Minimum macOS 11
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
    endif()
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    set(IS_DEBUG_BUILD ON)
    set(IS_RELEASE_BUILD OFF)
else()
    set(IS_DEBUG_BUILD OFF)
    set(IS_RELEASE_BUILD ON)
endif()

# LTO takes too much memory and time using MSVC.
if (NOT MSVC AND IS_RELEASE_BUILD)
    set(DEFAULT_ENABLE_LTO ON)
else()
    set(DEFAULT_ENABLE_LTO OFF)
endif()

CMAKE_DEPENDENT_OPTION(ENABLE_SOFTWARE_RENDERER "Enables the software renderer" ON "NOT ANDROID" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_OPENGL "Enables the OpenGL renderer" ON "NOT APPLE" OFF)
option(ENABLE_VULKAN "Enables the Vulkan renderer" ON)

# Compile options
CMAKE_DEPENDENT_OPTION(COMPILE_WITH_DWARF "Add DWARF debugging information" ${IS_DEBUG_BUILD} "MINGW" OFF)
option(ENABLE_LTO "Enable link time optimization" ${DEFAULT_ENABLE_LTO})
option(ENCORE_USE_PRECOMPILED_HEADERS "Use precompiled headers" ON)
option(ENCORE_WARNINGS_AS_ERRORS "Enable warnings as errors" ON)

include(EncoreHandleSystemLibs)

if (ENCORE_USE_PRECOMPILED_HEADERS)
    message(STATUS "Using Precompiled Headers.")
    set(CMAKE_PCH_INSTANTIATE_TEMPLATES ON)

    # This ensures that pre-compiled headers won't invalidate build caches for every fresh checkout.
    if(NOT MSVC AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND CMAKE_CXX_COMPILE_OPTIONS_CREATE_PCH -Xclang -fno-pch-timestamp)
    endif()
endif()

if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/.git AND NOT EXISTS ${PROJECT_SOURCE_DIR}/.git/hooks/pre-commit)
    message(STATUS "Copying pre-commit hook")
    file(COPY hooks/pre-commit
        DESTINATION ${PROJECT_SOURCE_DIR}/.git/hooks)
endif()

# Use ccache for android if available
# =======================================================================
if (NOT $ENV{NDK_CCACHE} EQUAL "")
    set(CCACHE_EXE $ENV{NDK_CCACHE})
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_EXE})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXE})
endif()

# Check for LTO support
# =======================================================================
if (ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error_msg)

    if (supported)
        message(STATUS "LTO enabled")
    else()
        message(STATUS "LTO enabled but is unavailable, disabling: ${error_msg}")
        set(ENABLE_LTO OFF)
    endif()
else()
    message(STATUS "LTO disabled")
endif()

if (ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Sanity check : Check that all submodules are present
# =======================================================================

function(check_submodules_present)
    file(READ "${PROJECT_SOURCE_DIR}/.gitmodules" gitmodules)
    string(REGEX MATCHALL "path *= *[^ \t\r\n]*" gitmodules ${gitmodules})
    foreach(module ${gitmodules})
        string(REGEX REPLACE "path *= *" "" module ${module})
        if (NOT EXISTS "${PROJECT_SOURCE_DIR}/${module}/.git")
            message(SEND_ERROR "Git submodule ${module} not found."
                    "Please run: git submodule update --init --recursive")
        endif()
    endforeach()
endfunction()
if (EXISTS "${PROJECT_SOURCE_DIR}/.git/objects")
    # only check submodules when source is obtained via Git
    check_submodules_present()
endif()

# Detect current compilation architecture and create standard definitions
# =======================================================================

include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE)
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
        endif()
    endif()
endfunction()

if (NOT ENABLE_GENERIC)
    if (CMAKE_OSX_ARCHITECTURES)
        set(ARCHITECTURE "${CMAKE_OSX_ARCHITECTURES}")
    elseif (MSVC)
        detect_architecture("_M_AMD64" x86_64)
        detect_architecture("_M_IX86" x86)
        detect_architecture("_M_ARM" arm)
        detect_architecture("_M_ARM64" arm64)
    else()
        detect_architecture("__x86_64__" x86_64)
        detect_architecture("__i386__" x86)
        detect_architecture("__arm__" arm)
        detect_architecture("__aarch64__" arm64)
    endif()
endif()
if (NOT DEFINED ARCHITECTURE)
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "Target architecture: ${ARCHITECTURE}")


# Configure C++ standard
# ===========================

# boost asio's concept usage doesn't play nicely with some compilers yet.
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)
# boost can have issues compiling with C++17 and up on newer versions of Clang.
add_definitions(-DBOOST_NO_CXX98_FUNCTION_BASE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Apply consistent visibility settings.
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# set up output paths for executable binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin/$<CONFIG>)


# System imported libraries
# ======================

# Prefer the -pthread flag on Linux.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Platform-specific library requirements
# ======================================

if (APPLE)
    if (NOT IOS)
        # Umbrella framework for everything GUI-related
        find_library(COCOA_LIBRARY Cocoa REQUIRED)
    endif()
    find_library(AVFOUNDATION_LIBRARY AVFoundation REQUIRED)
    find_library(IOSURFACE_LIBRARY IOSurface REQUIRED)
    set(PLATFORM_LIBRARIES ${COCOA_LIBRARY} ${AVFOUNDATION_LIBRARY} ${IOSURFACE_LIBRARY} ${MOLTENVK_LIBRARY})

    if (ENABLE_VULKAN)
        if (NOT USE_SYSTEM_MOLTENVK)
            download_moltenvk()
        endif()
        find_library(MOLTENVK_LIBRARY MoltenVK REQUIRED)
        message(STATUS "Using MoltenVK at ${MOLTENVK_LIBRARY}.")
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} ${MOLTENVK_LIBRARY})
    endif()
elseif (WIN32)
    set(PLATFORM_LIBRARIES winmm ws2_32)
    if (MINGW)
        # PSAPI is the Process Status API
        set(PLATFORM_LIBRARIES ${PLATFORM_LIBRARIES} psapi imm32 version)
    endif()
elseif (CMAKE_SYSTEM_NAME MATCHES "^(Linux|kFreeBSD|GNU|SunOS)$")
    set(PLATFORM_LIBRARIES rt)
endif()

# Setup a custom clang-format target (if clang-format can be found) that will run
# against all the src files. This should be used before making a pull request.
# =======================================================================

find_program(CLANG_FORMAT
    NAMES clang-format-15
          clang-format-16
          clang-format-17
          clang-format)

if (NOT CLANG_FORMAT)
    message(WARNING "Clang format not found! Disabling the clang format target")
endif()

if (CLANG_FORMAT)
    set(SRCS ${PROJECT_SOURCE_DIR}/src)
    set(CCOMMENT "Running clang format against all the .h and .cpp files in src/")
    if (WIN32)
        if(MINGW)
            add_custom_target(clang-format
                COMMAND find `cygpath -u ${SRCS}` -iname *.h -o -iname *.cpp -o -iname *.mm | xargs `cygpath -u ${CLANG_FORMAT}` -i
                COMMENT ${CCOMMENT})
        else()
            add_custom_target(clang-format
                COMMAND powershell.exe -Command "Get-ChildItem '${SRCS}/*' -Include *.cpp,*.h,*.mm -Recurse | Foreach {&'${CLANG_FORMAT}' -i $_.fullname}"
                COMMENT ${CCOMMENT})
        endif()
    else()
        add_custom_target(clang-format
            COMMAND find ${SRCS} -iname *.h -o -iname *.cpp -o -iname *.mm | xargs ${CLANG_FORMAT} -i
            COMMENT ${CCOMMENT})
    endif()
    unset(SRCS)
    unset(CCOMMENT)
endif()

# Include source code
# ===================

# This function should be passed a list of all files in a target. It will automatically generate
# file groups following the directory hierarchy, so that the layout of the files in IDEs matches the
# one in the filesystem.
function(create_target_directory_groups target_name)
    # Place any files that aren't in the source list in a separate group so that they don't get in
    # the way.
    source_group("Other Files" REGULAR_EXPRESSION ".")

    get_target_property(target_sources "${target_name}" SOURCES)

    foreach(file_name IN LISTS target_sources)
        get_filename_component(dir_name "${file_name}" PATH)
        # Group names use '\' as a separator even though the entire rest of CMake uses '/'...
        string(REPLACE "/" "\\" group_name "${dir_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

# Gets a UTC timstamp and sets the provided variable to it
function(get_timestamp _var)
    string(TIMESTAMP timestamp UTC)
    set(${_var} "${timestamp}" PARENT_SCOPE)
endfunction()

# generate git/build information
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REF_SPEC GIT_REV)
git_describe(GIT_DESC --always --long --dirty)
git_branch_name(GIT_BRANCH)
get_timestamp(BUILD_DATE)

# Boost
# Prevent boost from linking against libs when building
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY
    -DBOOST_SYSTEM_NO_LIB
    -DBOOST_DATE_TIME_NO_LIB
    -DBOOST_REGEX_NO_LIB
)

enable_testing()
add_subdirectory(externals)

# Boost (bundled)
add_definitions( -DBOOST_ALL_NO_LIB )
add_library(Boost::boost ALIAS boost)
add_library(Boost::serialization ALIAS boost_serialization)
add_library(Boost::iostreams ALIAS boost_iostreams)

add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT encore)
