// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file process.hpp
/// Includes the Boost.Process v1 API. Boost.Process moved its v1 API under
/// boost/process/v1/ starting with Boost 1.86, when the new Process v2 API
/// was introduced.

#pragma once

#include <boost/version.hpp>
#if BOOST_VERSION >= 108600
#include <boost/process/v1.hpp>
#include <boost/process/v1/async_pipe.hpp>
#include <boost/process/v1/extend.hpp>
#else
#include <boost/process.hpp>
#include <boost/process/async_pipe.hpp>
#include <boost/process/extend.hpp>
#endif
