// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread.cpp
/// Contains implementation of the thread class.

#include "thread.hpp"
#include "target.hpp"


namespace cxxdbg::dbg::core {


thread::thread(target & targ, size_t idx):
thread_base{targ, idx},
targ_{targ} {
}


stack_frame thread::frame(std::size_t i) const {
    return stack_frame{*this, i};
}


stack_frame thread::selected_frame() const {
    return frame(selected_frame_index());
}


}
