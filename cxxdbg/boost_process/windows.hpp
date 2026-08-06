// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file windows.hpp
/// Includes the Windows-specific extensions of the Boost.Process v1 API.
/// See process.hpp for background on the v1/v2 split.

#pragma once

#include <boost/version.hpp>
#if BOOST_VERSION >= 108600
#include <boost/process/v1/windows.hpp>
#else
#include <boost/process/windows.hpp>
#endif
