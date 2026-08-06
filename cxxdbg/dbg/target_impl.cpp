// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_impl.cpp
/// Contains implementation of target_impl class.

#include "target_impl.hpp"
#include <cassert>


namespace cxxdbg::dbg {


std::string target_impl::state_name(state_t st) {
    switch (st) {
    case state_t::invalid:      return "invalid";
    case state_t::loaded:       return "loaded";
    case state_t::launching:    return "launching";
    case state_t::running:      return "running";
    case state_t::stopped:      return "stopped";
    case state_t::terminating:  return "terminating";
    case state_t::detaching:    return "detaching";
    case state_t::unloading:    return "unloading";
    default:
        assert(false && "unknown state");
        return "unknown";
    }
}


}
