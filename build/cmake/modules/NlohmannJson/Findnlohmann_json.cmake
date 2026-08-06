# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

# thirdparty/CMakeLists.txt already builds the vendored nlohmann/json
# submodule unconditionally, so by the time find_package(nlohmann_json) is
# called elsewhere in the tree the real target already exists. This module
# just confirms that.

if(TARGET nlohmann_json::nlohmann_json)
    set(nlohmann_json_FOUND TRUE)
endif()
