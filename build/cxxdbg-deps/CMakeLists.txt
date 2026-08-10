# Project for building all CXXDBG dependencies

cmake_minimum_required(VERSION 3.20)

include(FetchContent)

# Selecting default toolchain and settings depending on platform
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/toolchain.cmake")

project(cxxdbg-deps)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake")

# Options for including/excluding components from build

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CXXDBG_BUILD_NCURSES_DEFAULT OFF)
    set(CXXDBG_BUILD_LIBEDIT_DEFAULT OFF)
    set(CXXDBG_BUILD_ZLIB_DEFAULT ON)
else()
    # TODO: enable ncurses and libedit on non Windows platforms
    set(CXXDBG_BUILD_NCURSES_DEFAULT OFF)
    set(CXXDBG_BUILD_LIBEDIT_DEFAULT OFF)
    set(CXXDBG_BUILD_ZLIB_DEFAULT OFF)
endif()


option(CXXDBG_BUILD_LZMA "Build lzma library from sources" ON)
option(CXXDBG_BUILD_LIBXML2 "Build libxml2 library from sources" ON)
option(CXXDBG_BUILD_NCURSES "Build ncurses library form sources" "${CXXDBG_BUILD_NCURSES_DEFAULT}")
option(CXXDBG_BUILD_LIBEDIT "Build libedit library form sources" "${CXXDBG_BUILD_LIBEDIT_DEFAULT}")
option(CXXDBG_BUILD_BOOST "Build boost library form sources" ON)
option(CXXDBG_BUILD_ZLIB "Build zlib library form sources" "${CXXDBG_BUILD_ZLIB_DEFAULT}")


# Setting global BUILD_SHARED_LIBS flag for Development deployment to build shared libraries
if("${CXXDBG_DEPLOYMENT_TYPE}" STREQUAL "Development")
    set(BUILD_SHARED_LIBS ON)
else()
    set(BUILD_SHARED_LIBS OFF)
endif()


# build all libraries with -fPIC on Posix
if(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    add_compile_options(-fPIC)
endif()


# Zlib
if("${CXXDBG_BUILD_ZLIB}")
    FetchContent_Declare(
        ZLIB
        GIT_REPOSITORY "https://github.com/madler/zlib.git"
        GIT_TAG "v1.3.1"
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(ZLIB)
endif()


# LZMA
if("${CXXDBG_BUILD_LZMA}")
    FetchContent_Declare(
        LibLZMA
        GIT_REPOSITORY "https://github.com/cxxdbg/xz.git"
        GIT_TAG "master"
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(LibLZMA)

    if(NOT TARGET LibLZMA::LibLZMA)
        add_library(LibLZMA::LibLZMA ALIAS liblzma)
    endif()
endif()


# libxml2
if("${CXXDBG_BUILD_LIBXML2}")
    set(LIBXML2_WITH_PYTHON OFF CACHE BOOL "Enable python in libxml2")
    set(LIBXML2_WITH_ICONV OFF CACHE BOOL "Enable iconv support in libxml2")
    set(LIBXML2_WITH_ICU OFF CACHE BOOL "Enable icu support in libxml2")

    FetchContent_Declare(
        LibXml2
        GIT_REPOSITORY "https://github.com/GNOME/libxml2.git"
        GIT_TAG "v2.13.8"
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(LibXml2)
endif()


# Libedit
if("${CXXDBG_BUILD_LIBEDIT}")
    FetchContent_Declare(
        LibEdit
        GIT_REPOSITORY "https://github.com/cxxdbg/libedit.git"
        GIT_TAG "libedit-20250104-3.1"
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(LibEdit)
endif()


# Ncurses
if("${CXXDBG_BUILD_NCURSES}")
    set(BOOST_SKIP_INSTALL_RULES OFF CACHE BOOL "Skip boost install")
    FetchContent_Declare(
        Curses
        GIT_REPOSITORY "https://github.com/cxxdbg/ncurses.git"
        GIT_TAG "master-cmake"
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(Curses)
endif()


# Boost
if("${CXXDBG_BUILD_BOOST}")
    set(BOOST_SKIP_INSTALL_RULES OFF CACHE BOOL "Skip boost install")

    # Disable ASM implementation in boost_context on Macos
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
        set(BOOST_CONTEXT_IMPLEMENTATION "ucontext" CACHE STRING "boost context impl")
    endif()

    FetchContent_Declare(
        Boost
        URL "https://github.com/boostorg/boost/releases/download/boost-1.85.0/boost-1.85.0-cmake.tar.gz"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        OVERRIDE_FIND_PACKAGE
    )
    FetchContent_MakeAvailable(Boost)

    # Creating Boost::boost target manually and making it alias
    # to used header only boost libraries
    add_library(Boost_boost INTERFACE)
    target_link_libraries(Boost_boost INTERFACE Boost::variant
                                                Boost::algorithm
                                                Boost::tuple
                                                Boost::property_tree
                                                Boost::asio
                                                Boost::logic
                                                Boost::iterator
                                                Boost::signals2
                                                Boost::dll
                                                Boost::process)
    add_library(Boost::boost ALIAS Boost_boost)
endif()
