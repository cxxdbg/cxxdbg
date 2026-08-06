// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_base.hpp
/// Contains definition of thread_base class.

#pragma once

#include "stop_reason.hpp"
#include <lldb/API/SBThread.h>


namespace cxxdbg::dbg::core {

class stack_frame_base;
class target_base;


/// \class const_thread
/// Represents single const thread in target being debugged
class const_thread {
    friend class stack_frame_base;

public:
    /// Constructor, makes thread with specified pointer to LLDB thread
    const_thread(target_base & targ, const lldb::SBThread & thrd);

    /// Copy constructor
    const_thread(const const_thread &);

    /// Destructor, destroys object
    ~const_thread();

    /// Returns thread id
    unsigned long id() const;

    /// Returns index of selected frame
    std::size_t selected_frame_index() const;

    /// Returns thread stop reason
    stop_reason_t stop_reason() const;

    /// Returns reference to target this thread belongs to
    target_base & targ() const { return targ_; }

protected:
    /// Returns pointer to LLDB thread
    lldb::SBThread thrd() const;

private:
    target_base & targ_;        ///< Reference to target
    lldb::SBThread thrd_;       ///< Pointer to LLDB thread
};


/// \class thread_base
/// Represents single thread in target being debugged. Base class for the thread class,
/// contains code dependent from LLDB.
class thread_base: public const_thread {
public:
    /// Constructs thread for target and index of thread in target
    thread_base(target_base & targ, size_t idx);

    /// Destructor, destroys object
    ~thread_base();

    /// Sets selected frame index
    void set_selected_frame(std::size_t i) const;
};


}
