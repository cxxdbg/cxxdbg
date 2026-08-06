// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file compile_unit_info.cpp
/// Contains implementation of compile_unit_info class.

#include "compile_unit_info.hpp"


namespace cxxdbg::dbg {


compile_unit_info::compile_unit_info(const std::filesystem::path & src):
source_(src) {
}


const std::filesystem::path & compile_unit_info::source_path() const {
    return source_;
}


}
