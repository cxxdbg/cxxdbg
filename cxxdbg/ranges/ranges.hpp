// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// Ranges library main header for inclusion. Uses range-v3 library or
/// the standard ranges library depending on CXXDBG_USE_RANGE_V3 maco.


#pragma once

#ifdef CXXDBG_USE_RANGE_V3

#include <range/v3/all.hpp>

#else

// The iosfwd include is workaround for the QTBUG-73263 and QTBUG-83160 bugs that cause moc to fail to
// precompile headers that include <filesystem> or <ranges> header. The <ioswfd> include fixes the problem.
// See https://bugreports.qt.io/browse/QTBUG-73263 and https://bugreports.qt.io/browse/QTBUG-83160for more details.
#include <iosfwd>

#include <ranges>
#include <algorithm>

#endif

#include "concat.hpp"
#include "join_delim.hpp"
