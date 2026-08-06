
cmake_minimum_required(VERSION 3.0)

# Selecting default toolchain and settings depending on platform
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/toolchain.cmake")

# Standalone (without LLVM) build of LLDB for CXXDBG
project(cxxdbg-lldb)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake")


if(CXXDBG_USE_LLD)
    set(LLVM_USE_LINKER "lld")
elseif(NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(LLVM_USE_LINKER "gold")
endif()


# Use -Og for debug build on Windows with gcc because of assembler file overflow
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU" AND MINGW)
    set(CMAKE_C_FLAGS_DEBUG "-Og -g")
    set(CMAKE_CXX_FLAGS_DEBUG "-Og -g")
endif()


# LLVM configuration and build

set(LLDB_ENABLE_PYTHON OFF CACHE BOOL "Enable python")
set(LLVM_ENABLE_RTTI ON CACHE BOOL "Enable LLVM RTTI")
set(LLDB_ENABLE_STANDALONE ON CACHE BOOL "Standalone LLBB build")
set(LLDB_BUILD_STATIC ON CACHE BOOL "Build static liblldb")

if("${CXXDBG_LLVM_ENABLE_SHARED}")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Enable shared libraries in LLVM")
endif()

if(NOT "${CXXDBG_LLVM_ENABLE_SHARED}")
    set(CMAKE_C_FLAGS "-fvisibility=hidden ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "-fvisibility=hidden ${CMAKE_CXX_FLAGS}")
endif()


# disable memcpy related warning becuse gcc-8 generates it for core LLVM headers
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb" "lldb")


# install all Plugins headers
file(GLOB_RECURSE headers
     LIST_DIRECTORIES false
     RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/source"
     "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/source/Plugins/*.h")
foreach(header ${headers})
    set(input "${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/lldb/source/${header}")
    get_filename_component(prefix "${header}" DIRECTORY)
    install(FILES "${input}" DESTINATION "include/${prefix}")
endforeach()

