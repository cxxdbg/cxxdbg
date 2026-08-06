// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file pointer_value_impl.cpp
/// Contains implementation of pointer_value_impl class

#include "pointer_type_impl.hpp"
#include "source_position.hpp"
#include "target_base.hpp"
#include "utils.hpp"
#include "dbgfmt/backend/format_error.hpp"
#include <lldb/API/SBAddress.h>
#include <lldb/API/SBDeclaration.h>
#include <lldb/API/SBError.h>
#include <lldb/API/SBFunction.h>
#include <cstring>


namespace cxxdbg::dbg::core {


pointer_type_impl::pointer_type_impl(target_base & targ, const lldb::SBType & t):
core::type_impl(targ, t) {
}


pointer_type_impl::~pointer_type_impl() {
}


}
