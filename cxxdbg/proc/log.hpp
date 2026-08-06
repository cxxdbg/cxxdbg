// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains commong definitions for logging in process library

#pragma once

#include "cxxdbg/log/log.hpp"


#define CXXDBG_PROC_LOG_TRACE     CXXDBG_LOG_TRACE(proc)
#define CXXDBG_PROC_LOG_DEBUG     CXXDBG_LOG_DEBUG(proc)
#define CXXDBG_PROC_LOG_INFO      CXXDBG_LOG_INFO(proc)
#define CXXDBG_PROC_LOG_WARNING   CXXDBG_LOG_WARNING(proc)
#define CXXDBG_PROC_LOG_ERROR     CXXDBG_LOG_ERROR(proc)
#define CXXDBG_PROC_LOG_FATAL     CXXDBG_LOG_FATAL(proc)
