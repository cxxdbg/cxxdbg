# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Contains CXXDBG version information

set(CXXDBG_VERSION_MAJOR          1)
set(CXXDBG_VERSION_MINOR          7)
set(CXXDBG_VERSION_PATCHLEVEL     0)

# default version name is major.minor.patchlevel
set(CXXDBG_DEFAULT_VERSION_NAME "${CXXDBG_VERSION_MAJOR}.${CXXDBG_VERSION_MINOR}.${CXXDBG_VERSION_PATCHLEVEL}")
set(CXXDBG_VERSION_IS_RELEASE OFF)

if("${CXXDBG_VERSION_NAME}" STREQUAL "")
    # version name is not set, use default
    set(CXXDBG_VERSION_NAME "${CXXDBG_DEFAULT_VERSION_NAME}")

    # set is_release flag
    set(CXXDBG_VERSION_IS_RELEASE "true")
else()
    set(CXXDBG_VERSION_IS_RELEASE "false")
endif()
