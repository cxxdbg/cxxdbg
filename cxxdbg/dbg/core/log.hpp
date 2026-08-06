// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains definitions of functions and macroses for logging in core library.

#pragma once

#include "cxxdbg/log/log.hpp"


#define CXXDBG_CORE_LOG_TRACE       CXXDBG_LOG_TRACE(core)
#define CXXDBG_CORE_LOG_DEBUG       CXXDBG_LOG_DEBUG(core)
#define CXXDBG_CORE_LOG_INFO        CXXDBG_LOG_INFO(core)
#define CXXDBG_CORE_LOG_WARNING     CXXDBG_LOG_WARNING(core)
#define CXXDBG_CORE_LOG_ERROR       CXXDBG_LOG_ERROR(core)
#define CXXDBG_CORE_LOG_FATAL       CXXDBG_LOG_FATAL(core)
