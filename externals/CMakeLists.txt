# Definitions for all external bundled libraries

# Suppress warnings from external libraries
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W0)
else()
    add_compile_options(-w)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/CMakeModules)
include(DownloadExternals)
include(ExternalProject)

# Boost
message(STATUS "Including vendored Boost library")
set(BOOST_ROOT "${CMAKE_SOURCE_DIR}/externals/boost" CACHE STRING "")
set(Boost_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/externals/boost" CACHE STRING "")
set(Boost_NO_SYSTEM_PATHS ON CACHE BOOL "")
add_library(boost INTERFACE)
target_include_directories(boost SYSTEM INTERFACE ${Boost_INCLUDE_DIR})

# Boost::serialization
file(GLOB boost_serialization_SRC "${CMAKE_SOURCE_DIR}/externals/boost/libs/serialization/src/*.cpp")
add_library(boost_serialization STATIC ${boost_serialization_SRC})
target_link_libraries(boost_serialization PUBLIC boost)

# Boost::iostreams
add_library(
    boost_iostreams
    STATIC
    ${CMAKE_SOURCE_DIR}/externals/boost/libs/iostreams/src/file_descriptor.cpp
    ${CMAKE_SOURCE_DIR}/externals/boost/libs/iostreams/src/mapped_file.cpp
)
target_link_libraries(boost_iostreams PUBLIC boost)

# Crypto++
if (WIN32 AND NOT MSVC AND "arm64" IN_LIST ARCHITECTURE)
    # TODO: CryptoPP ARM64 ASM does not seem to support Windows unless compiled with MSVC.
    # TODO: See https://github.com/weidai11/cryptopp/issues/1260
    set(CRYPTOPP_DISABLE_ASM ON CACHE BOOL "")
endif()

set(CRYPTOPP_BUILD_DOCUMENTATION OFF CACHE BOOL "")
set(CRYPTOPP_BUILD_TESTING OFF CACHE BOOL "")
set(CRYPTOPP_INSTALL OFF CACHE BOOL "")
set(CRYPTOPP_SOURCES "${CMAKE_SOURCE_DIR}/externals/cryptopp" CACHE STRING "")
add_subdirectory(cryptopp-cmake)

if (MSVC)
    target_compile_definitions(cryptopp PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()

# dds-ktx
add_library(dds-ktx INTERFACE)
target_include_directories(dds-ktx INTERFACE ./dds-ktx)

# fmt and Xbyak need to be added before dynarmic
# libfmt
option(FMT_INSTALL "" ON)
add_subdirectory(fmt EXCLUDE_FROM_ALL)


# Xbyak
if ("x86_64" IN_LIST ARCHITECTURE)
    add_subdirectory(xbyak EXCLUDE_FROM_ALL)
endif()

# Oaknut
if ("arm64" IN_LIST ARCHITECTURE)
    add_subdirectory(oaknut EXCLUDE_FROM_ALL)
endif()

# Dynarmic
if ("x86_64" IN_LIST ARCHITECTURE OR "arm64" IN_LIST ARCHITECTURE)
    set(DYNARMIC_TESTS OFF CACHE BOOL "")
    set(DYNARMIC_FRONTENDS "A32" CACHE STRING "")
    set(DYNARMIC_USE_PRECOMPILED_HEADERS ${ENCORE_USE_PRECOMPILED_HEADERS} CACHE BOOL "")
    add_subdirectory(dynarmic EXCLUDE_FROM_ALL)
endif()

# MicroProfile
add_library(microprofile INTERFACE)
target_include_directories(microprofile SYSTEM INTERFACE ./microprofile)

# Nihstro
add_library(nihstro-headers INTERFACE)
target_include_directories(nihstro-headers SYSTEM INTERFACE ./nihstro/include)
if (MSVC)
    # TODO: For some reason MSVC still applies this warning even with /W0 for externals.
    target_compile_options(nihstro-headers INTERFACE /wd4715)
endif()

# Open Source Archives
add_subdirectory(open_source_archives)

# faad2
add_subdirectory(faad2 EXCLUDE_FROM_ALL)

# SoundTouch
set(INTEGER_SAMPLES ON CACHE BOOL "")
set(SOUNDSTRETCH OFF CACHE BOOL "")
set(SOUNDTOUCH_DLL OFF CACHE BOOL "")
add_subdirectory(soundtouch EXCLUDE_FROM_ALL)
target_compile_definitions(SoundTouch PUBLIC SOUNDTOUCH_INTEGER_SAMPLES)

# Teakra
add_subdirectory(teakra EXCLUDE_FROM_ALL)

# Zstandard
set(ZSTD_LEGACY_SUPPORT OFF)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_SHARED OFF)
add_subdirectory(zstd/build/cmake EXCLUDE_FROM_ALL)
target_include_directories(libzstd_static INTERFACE $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/externals/zstd/lib>)
add_library(zstd ALIAS libzstd_static)

# JSON
add_library(json-headers INTERFACE)
target_include_directories(json-headers SYSTEM INTERFACE ./json)

# LibreSSL
set(LIBRESSL_SKIP_INSTALL ON CACHE BOOL "")
set(OPENSSLDIR "/etc/ssl/")
add_subdirectory(libressl EXCLUDE_FROM_ALL)
target_include_directories(ssl SYSTEM INTERFACE ./libressl/include)
target_compile_definitions(ssl PRIVATE -DHAVE_INET_NTOP)
get_directory_property(OPENSSL_LIBRARIES
    DIRECTORY libressl
    DEFINITION OPENSSL_LIBS)

# httplib
add_library(httplib INTERFACE)
target_include_directories(httplib SYSTEM INTERFACE ./httplib)
target_compile_options(httplib INTERFACE -DCPPHTTPLIB_OPENSSL_SUPPORT)
target_link_libraries(httplib INTERFACE ${OPENSSL_LIBRARIES})

# lodepng
add_subdirectory(lodepng)

# OpenGL dependencies
if (ENABLE_OPENGL)
    # Glad
    add_subdirectory(glad)
endif()

# Vulkan dependencies
if (ENABLE_VULKAN)
    # glslang
    set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")
    set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
    set(ENABLE_SPVREMAPPER OFF CACHE BOOL "")
    set(ENABLE_CTEST OFF CACHE BOOL "")
    set(ENABLE_HLSL OFF CACHE BOOL "")
    set(BUILD_EXTERNAL OFF CACHE BOOL "")
    add_subdirectory(glslang)

    # sirit
    add_subdirectory(sirit EXCLUDE_FROM_ALL)

    # VMA
    add_library(vma INTERFACE)
    target_include_directories(vma SYSTEM INTERFACE ./vma/include)

    # vulkan-headers
    add_library(vulkan-headers INTERFACE)
    target_include_directories(vulkan-headers SYSTEM INTERFACE ./vulkan-headers/include)
endif()

# blip_buf
add_subdirectory(blip_buf)
