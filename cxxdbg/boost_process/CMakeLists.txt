# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Small helper library that centralizes the Boost version check for
# including Boost.Process (see process.hpp).
add_library(cxxdbg-boost-process INTERFACE)

target_link_libraries(cxxdbg-boost-process INTERFACE Boost::process)
