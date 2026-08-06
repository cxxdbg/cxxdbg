// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file system_monitor.hpp
/// Contains definition of the system_monitor type alias.

#pragma once

#ifdef _WIN32
#include "win32_monitor.hpp"
#else
#include "posix_monitor.hpp"
#endif


namespace cxxdbg::proc {


/// External process minitor that uses system specific API to monitor external
/// (not necessary child) process status.
#ifdef _WIN32
using system_monitor = win32_monitor;
#else
using system_monitor = posix_monitor;
#endif


}
