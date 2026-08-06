// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file thread_processor.hpp
/// Contains definition of the thread_processor class.

#pragma once

#include "cxxdbg/async/forward.hpp"
#include <boost/logic/tribool.hpp>


namespace cxxdbg::dbg::cli {


/// Abstract processor of commands from thread group
class thread_processor {
public:
    typedef boost::tribool tribool;

    /// Destructor, destroys object
    virtual ~thread_processor() {}

    /// Selects active thread
    virtual void select_thread(std::size_t index, const async::result_handler<> & h) = 0;

    /// Performs step into action
    virtual void step_into(tribool avoid_nodebug,
                           const std::string & target_func,
                           const std::string & avoid_regex,
                           const std::string & step_through_regex,
                           const async::result_handler<> & h) = 0;

    /// Performs step over action
    virtual void step_over(const std::string & step_through_regex,
                           const async::result_handler<> & h) = 0;

    /// Performs step out action
    virtual void step_out(tribool avoid_nodebug,
                          const std::string & step_through_regex,
                          const async::result_handler<> & h) = 0;

    /// Performs instruction step into action
    virtual void inst_step_into(const async::result_handler<> & h) = 0;

    /// Performs instruction step over action
    virtual void inst_step_over(const async::result_handler<> & h) = 0;

    /// Performs step until action
    virtual void step_until(unsigned int line, const async::result_handler<> & h) = 0;
};


}


