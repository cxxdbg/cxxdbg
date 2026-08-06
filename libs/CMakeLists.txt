# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# In-house libraries developed alongside cxxdbg and vendored here as git
# submodules. Unlike thirdparty/ (external third-party dependencies), these
# are cxxdbg's own libraries, split out into their own repositories so they
# can be reused/built standalone.

# cxxdap is a vendored git submodule with its own standalone CMake project,
# not header-only. Same BUILD_SHARED_LIBS convention as cm (see
# thirdparty/CMakeLists.txt); install_cxxdbg_library() is called here rather
# than from cxxdap's own CMakeLists.txt, since that function is a
# cxxdbg-specific convention cxxdap's standalone build doesn't have.
set(BUILD_SHARED_LIBS "${CXXDBG_ENABLE_SHARED}")
add_subdirectory(cxxdap)
unset(BUILD_SHARED_LIBS)

install_cxxdbg_library(cxxdap)

# cxxtn is a vendored git submodule with its own standalone CMake project,
# not header-only. Same BUILD_SHARED_LIBS convention as cxxdap above;
# install_cxxdbg_library() is called here rather than from cxxtn's own
# CMakeLists.txt, since that function is a cxxdbg-specific convention
# cxxtn's standalone build doesn't have.
set(BUILD_SHARED_LIBS "${CXXDBG_ENABLE_SHARED}")
add_subdirectory(cxxtn)
unset(BUILD_SHARED_LIBS)

install_cxxdbg_library(cxxtn)

# dbgfmt is a vendored git submodule with its own standalone CMake project,
# not header-only. Depends on cm/cm-cxx (added in thirdparty/CMakeLists.txt)
# and cxxtn (added above). Same BUILD_SHARED_LIBS convention as
# cxxdap/cxxtn above; install_cxxdbg_library() is called here rather than
# from dbgfmt's own CMakeLists.txt, since that function is a cxxdbg-specific
# convention dbgfmt's standalone build doesn't have.
# DBGFMT_FETCH_CXXTN/DBGFMT_FETCH_CM are disabled since cm and cxxtn are
# already provided by thirdparty/CMakeLists.txt's add_subdirectory(cm) and
# this file's own add_subdirectory(cxxtn); dbgfmt should use those existing
# targets rather than locating or fetching its own.
set(DBGFMT_FETCH_CXXTN OFF)
set(DBGFMT_FETCH_CM OFF)
set(BUILD_SHARED_LIBS "${CXXDBG_ENABLE_SHARED}")
add_subdirectory(dbgfmt)
unset(BUILD_SHARED_LIBS)

install_cxxdbg_library(cxxdbg-dbg-fmt)
install_cxxdbg_library(cxxdbg-dbg-fmtcommon)
install_cxxdbg_library(cxxdbg-dbg-fmtcore)
install_cxxdbg_library(cxxdbg-dbg-fmtcm)
install_cxxdbg_library(cxxdbg-data)
