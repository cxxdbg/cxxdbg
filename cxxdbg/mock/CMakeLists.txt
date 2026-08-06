# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# Mock framework library
add_library(cxxdbg-mock ${LIBRARY_TYPE}
            mock.cpp
            mock.hpp
           )

install_cxxdbg_library(cxxdbg-mock)
