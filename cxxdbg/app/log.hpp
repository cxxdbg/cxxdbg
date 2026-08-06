// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains commong definitions for logging in application

#pragma once

#include "cxxdbg/log/log.hpp"


#define CXXDBG_APP_LOG_TRACE       CXXDBG_LOG_TRACE(app)
#define CXXDBG_APP_LOG_DEBUG       CXXDBG_LOB_DEBUG(app)
#define CXXDBG_APP_LOG_INFO        CXXDBG_LOG_INFO(app)
#define CXXDBG_APP_LOG_WARNING     CXXDBG_LOG_WARNING(app)
#define CXXDBG_APP_LOG_ERROR       CXXDBG_LOG_ERROR(app)
#define CXXDBG_APP_LOG_FATAL       CXXDBG_LOG_FATAL(app)
