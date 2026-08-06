// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file frame_processor.hpp
/// Contains definition of the frame_processor class.

#pragma once

#include "cxxdbg/async/forward.hpp"


namespace cxxdbg::dbg::cli {


/// \class frame_processor
/// Processor for frame command group
class frame_processor {
public:
    /// Destructor, destroys object
    virtual ~frame_processor() {}

    /// Selects frame with specified index
    virtual void select_frame(std::size_t index, const async::result_handler<> &) = 0;
};


}


