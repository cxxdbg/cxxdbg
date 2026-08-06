// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_list_info.hpp
/// Contains definition of thread_list_info class.

#pragma once

#include "thread_info.hpp"
#include <vector>


namespace cxxdbg::dbg {


/// \class thread_list_info
/// Contains info about thread list in process being debugged
class thread_list_info {
    /// Type of vector of thread infos
    typedef std::vector<thread_info> thread_info_vector;

public:
    /// Type of const iterator over thread list
    typedef thread_info_vector::const_iterator const_iterator;

    /// Type of thread list size
    typedef thread_info_vector::size_type size_type;

    /// Invalid value for thread index
    static const size_type invalid_index = static_cast<size_type>(-1);


    /// Constructor, makes empty thread list info
    thread_list_info();

    /// Returns const iterator pointing to the first thread in list
    const_iterator begin() const;

    /// Returns const iterator pointing to the on past last thread in list
    const_iterator end() const;

    /// Returns thread info at index
    const thread_info & operator[](size_type i) const;

    /// Returns index of current thread, or invalid_index, if current
    /// thread is not set
    size_type current_thread() const;

    /// Adds thread to list of threads
    void add(const thread_info & thrd);

    /// Sets current thread index
    void set_current_thread(size_type i);

private:
    thread_info_vector threads_;        ///< List of threads
    size_type active_thread_;           ///< Number of active thread in list
};


}


