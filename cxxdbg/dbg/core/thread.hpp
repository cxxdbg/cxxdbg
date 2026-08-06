// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread.hpp
/// Contains definition of the thread class.

#pragma once

#include "stack_frame.hpp"
#include "thread_base.hpp"
#include <ranges.hpp>


namespace cxxdbg::dbg::core {

class target;


/// Represents single thread in target being debugged
class thread: public thread_base {
public:
    /// Constructs thread for target and index of thread in target
    thread(target & targ, size_t idx);

    /// Returns stack frame with specified index
    stack_frame frame(std::size_t i) const;

    /// Returns selected frame
    stack_frame selected_frame() const;

    /// Returns range of stack frames
    auto frames() const {
        // TODO: frames are now infinite range and all users must check
        // for invalid frames to break loops. We need refactor it.
        auto indexes = std::ranges::views::iota(size_t());
        auto fn = [this](size_t idx) { return frame(idx); };
        return indexes | std::ranges::views::transform(fn);
    }

    /// Returns reference to target this thread belongs to
    target & targ() const { return targ_; }

private:
    target & targ_;         ///< Reference to target this thread belongs to
};


}
