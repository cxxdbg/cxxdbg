
cmake_minimum_required(VERSION 3.0)

# Selecting default toolchain and settings depending on platform
include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/toolchain.cmake")

project(cxxdbg-llvm)

include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/common.cmake")

option(CXXDBG_BUILD_LLDB "Build LLDB together with LLVM" ON)


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

set(projects "clang")
if("${CXXDBG_BUILD_LLDB}")
    list(APPEND projects "lldb")
endif()

set(CLANG_ENABLE_CINDEX OFF CACHE BOOL "Enable CIndex")
set(CLANG_ENABLE_ARCMT OFF CACHE BOOL "Enable ARCMT")
set(LLDB_ENABLE_PYTHON OFF CACHE BOOL "Enable python")
set(LLVM_TARGETS_TO_BUILD "X86;ARM;AArch64" CACHE STRING "")
set(CLANG_INCLUDE_TESTS OFF CACHE BOOL "Enable clang tests")
set(LLDB_INCLUDE_TESTS OFF CACHE BOOL "Enable LLDB tests")
set(LLVM_ENABLE_PROJECTS "${projects}" CACHE STRING "")
set(LLVM_ENABLE_RTTI ON CACHE BOOL "Enable LLVM RTTI")
set(LLVM_ENABLE_LTO OFF CACHE BOOL "Enable LTO in LLVM")
set(LLVM_INSTALL_TOOLCHAIN_ONLY ON CACHE BOOL "Install only toolchain")
set(CLANG_TOOLING_BUILD_AST_INTROSPECTION OFF CACHE BOOL "Disable clang AST introspection")
set(LLVM_ENABLE_ZSTD OFF CACHE BOOL "Enable ZSTD")

if("${CXXDBG_LLVM_ENABLE_SHARED}")
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Enable shared libraries in LLVM")
    set(LLDB_EXPORT_ALL_SYMBOLS ON CACHE BOOL "Export all LLDB symbols")
endif()

if("${CXXDBG_LLVM_ENABLE_SHARED}")
    set(LLDB_BUILD_STATIC_LIBLLDB_val OFF)
else()
    set(LLDB_BUILD_STATIC_LIBLLDB_val ON)
endif()
set(LLDB_BUILD_STATIC_LIBLLDB "${LLDB_BUILD_STATIC_LIBLLDB_val}" CACHE BOOL "Build static liblldb")

if(NOT "${CXXDBG_LLVM_ENABLE_SHARED}" AND NOT "${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden ${CMAKE_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
endif()


# disable memcpy related warning becuse gcc-8 generates it for core LLVM headers
if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../llvm-project/llvm" "llvm")


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

