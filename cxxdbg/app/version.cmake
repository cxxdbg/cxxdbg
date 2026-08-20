# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Contains CXXDBG version information

set(CXXDBG_VERSION_MAJOR          0)
set(CXXDBG_VERSION_MINOR          1)
set(CXXDBG_VERSION_PATCHLEVEL     1)

# numeric version, major.minor.patchlevel
set(CXXDBG_VERSION_NUMBER "${CXXDBG_VERSION_MAJOR}.${CXXDBG_VERSION_MINOR}.${CXXDBG_VERSION_PATCHLEVEL}")
set(CXXDBG_VERSION_IS_RELEASE OFF)

if("${CXXDBG_VERSION_SUFFIX}" STREQUAL "")
    # no suffix set, this is a release build
    set(CXXDBG_VERSION_IS_RELEASE "true")
else()
    set(CXXDBG_VERSION_IS_RELEASE "false")
endif()
