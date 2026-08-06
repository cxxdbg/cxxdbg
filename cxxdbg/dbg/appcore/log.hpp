// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains definitions of functions and macroses for logging in appcore library.

#pragma once

#include "cxxdbg/log/log.hpp"


#define CXXDBG_APPCORE_LOG_TRACE       CXXDBG_LOG_TRACE(appcore)
#define CXXDBG_APPCORE_LOG_DEBUG       CXXDBG_LOG_DEBUG(appcore)
#define CXXDBG_APPCORE_LOG_INFO        CXXDBG_LOG_INFO(appcore)
#define CXXDBG_APPCORE_LOG_WARNING     CXXDBG_LOG_WARNING(appcore)
#define CXXDBG_APPCORE_LOG_ERROR       CXXDBG_LOG_ERROR(appcore)
#define CXXDBG_APPCORE_LOG_FATAL       CXXDBG_LOG_FATAL(appcore)
