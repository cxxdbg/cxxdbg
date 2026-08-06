// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_info.hpp
/// Contains definition of thread_info class.

#pragma once

#include "stack_frame_info.hpp"
#include <cstdint>
#include <vector>


namespace cxxdbg::dbg {


/// \class thread_info
/// Contains information about thread in process being debugged
class thread_info {
    /// Type of vector of stack frame infos
    typedef std::vector<stack_frame_info> stack_frame_info_vector;

    /// Type of const iterator over stack frames
    typedef stack_frame_info_vector::const_iterator const_stack_frame_iterator;

public:
    /// Constructor, makes thread info with specified id and empty call stack
    thread_info(std::uint64_t i);

    /// Returns thread id
    std::uint64_t id() const;

    /// Returns const reference to call stack
    const stack_frame_info_vector & call_stack() const;

    /// Returns iterator pointing to the first stack frame info in call stack
    const_stack_frame_iterator call_stack_begin() const;

    /// Returns iterator pointing to the one past last frame info in call stack
    const_stack_frame_iterator call_stack_end() const;

    /// Returns size of call stack
    size_t call_stack_size() const;

    /// Adds stack frame info into call stack
    void add_stack_frame(const stack_frame_info & finfo);

    /// Returns true if thread has complete call stack
    bool has_complete_call_stack() const;

    /// Sets complete call stack flag
    void set_has_complete_call_stack(bool v);

    /// Returns index of current frame
    size_t curr_frame_index() const;

    /// Sets index of current frame
    void set_curr_frame_index(size_t idx);

    /// Sets vector of call targets in current position
    void set_call_targets(const std::vector<std::string> & targs) {
        call_targets_ = targs;
    }

    /// Returns range of name of call targets in current thread location
    auto & call_targets() const { return call_targets_; }

private:
    std::uint64_t id_;                          ///< Thread ID
    stack_frame_info_vector call_stack_;        ///< Call stack info
    size_t curr_frame_ = 0;                     ///< Index of current frame
    bool is_complete_call_stack_;               ///< True if call stack is complete
    std::vector<std::string> call_targets_;     ///< Vector of call targets in current position
};


typedef std::shared_ptr<thread_info> thread_info_sp;


}


