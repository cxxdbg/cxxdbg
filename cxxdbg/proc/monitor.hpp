// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file monitor.hpp
/// Contains definition of the monitor class.

#pragma once

#include <string>
#include <boost/signals2/signal.hpp>


namespace cxxdbg::proc {


/// Abstract high level process monitor.
/// Notifies clients about process termination. Can terminate process on request.
class monitor {
public:
    /// Virtual destructor. Destroys process object.
    virtual ~monitor() = default;

    /// Starts process termination
    virtual void terminate() = 0;

    /// The signal is emitted when process exits
    boost::signals2::signal<void (int)> exited;
};


}
