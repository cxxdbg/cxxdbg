// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file log.hpp
/// Contains definition of logging functions and macroses for CXXDBG dap server

#pragma once

#include "cxxdbg/log/log.hpp"


#define CXXDBG_DAP_LOG_TRACE       CXXDBG_LOG_TRACE(cxxdbgdap)
#define CXXDBG_DAP_LOG_DEBUG       CXXDBG_LOG_DEBUG(cxxdbgdap)
#define CXXDBG_DAP_LOG_INFO        CXXDBG_LOG_INFO(cxxdbgdap)
#define CXXDBG_DAP_LOG_WARNING     CXXDBG_LOG_WARNING(cxxdbgdap)
#define CXXDBG_DAP_LOG_ERROR       CXXDBG_LOG_ERROR(cxxdbgdap)
#define CXXDBG_DAP_LOG_FATAL       CXXDBG_LOG_FATAL(cxxdbgdap)
