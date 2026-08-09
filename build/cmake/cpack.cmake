# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Script for generating CPack project for packaing cxxdbg into deb/rpm/msi/dmg packages


include("${CMAKE_CURRENT_SOURCE_DIR}/cxxdbg/app/version.cmake")

# system name (will be used as suffix for package file name)
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(CPACK_SYSTEM_NAME "linux")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(CPACK_SYSTEM_NAME "macos")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CPACK_SYSTEM_NAME "windows")
endif()

# selecting used generators
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
    set(CPACK_GENERATOR "TBZ2" "DEB" "RPM")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
    set(CPACK_GENERATOR "ZIP" "DragNDrop")
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    set(CPACK_GENERATOR "ZIP")
endif()


set(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR};${CMAKE_PROJECT_NAME};cxxdbg;/")
set(CPACK_PACKAGE_NAME "cxxdbg")
set(CPACK_PACKAGE_VENDOR "cxxdbg")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packages")
set(CPACK_PACKAGE_VERSION_MAJOR "${CXXDBG_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CXXDBG_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CXXDBG_VERSION_PATCHLEVEL}")
set(CPACK_PACKAGE_DESCRIPTION "Advanced C++ Debugger for Linux, Windows and Mac OS platforms.")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "CxxDbg")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://cxxdbg.org")
set(CPACK_PACKAGE_CONTACT "support@cxxdbg.org")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "cxxdbg")

# CPack's DEB generator normally auto-detects this via `dpkg
# --print-architecture`, but that can be unavailable in minimal build
# images, silently producing a package with an empty Architecture field.
if("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(x86_64|amd64)$")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
elseif("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "^(aarch64|arm64)$")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Post-build action for signing .dmg
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin"
   AND NOT "${LLDB_CODESIGN_IDENTITY}" STREQUAL "")
    set(CPACK_CXXDBG_CODESIGN_IDENTITY "${LLDB_CODESIGN_IDENTITY}")
    set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_CURRENT_LIST_DIR}/codesign_dmg.cmake")
endif()

include(CPack)
