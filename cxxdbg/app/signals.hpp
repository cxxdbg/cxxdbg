// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "cxxdbg/util/signals.hpp"


namespace cxxdbg {


using signal_connection = util::signal_connection;
using scoped_signal_connection = util::scoped_signal_connection;
template <typename Func> using signal = util::signal<Func>;


}


