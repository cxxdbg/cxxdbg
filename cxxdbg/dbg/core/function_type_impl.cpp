// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file function_type_impl.cpp
/// Contains implementation of the function_type_impl class.

#include "function_type_impl.hpp"
#include "target_base.hpp"


namespace cxxdbg::dbg::core {


const type_impl * function_type_impl::ret_type() const {
    return targ().get_or_create_type(lldb_type().GetFunctionReturnType());
}


size_t function_type_impl::params_size() const {
    return static_cast<size_t>(lldb_type().GetFunctionArgumentTypes().GetSize());
}


const type_impl * function_type_impl::param_at(size_t idx) const {
    auto pars = lldb_type().GetFunctionArgumentTypes();
    assert(idx < pars.GetSize() && "invalud function parameter index");
    auto par = pars.GetTypeAtIndex(static_cast<uint32_t>(idx));
    return targ().get_or_create_type(par);
}


}
