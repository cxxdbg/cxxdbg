// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file version.hpp
/// Contains cxxdbg version information

#pragma once

#include "version_config.hpp"


namespace cxxdbg {


/// Major version of CXXDBG
const int version_major = CXXDBG_VERSION_MAJOR;

/// Minor version of CXXDBG
const int version_minor = CXXDBG_VERSION_MINOR;

/// Patchlevel version of CXXDBG
const int version_patchlevel = CXXDBG_VERSION_PATCHLEVEL;

/// True if CXXDBG version is release version
const bool version_is_release = CXXDBG_VERSION_IS_RELEASE;


/// String value of CXXDBG major version
const char * const version_major_str = CXXDBG_VERSION_MAJOR_STR;

/// String value of CXXDBG minor version
const char * const version_minor_str = CXXDBG_VERSION_MINOR_STR;

/// String value of CXXDBG patchlevel version
const char * const version_patchlevel_str = CXXDBG_VERSION_PATCHLEVEL_STR;

/// String value of CXXDBG full version
const char * const version_full_str =
        CXXDBG_VERSION_MAJOR_STR "." CXXDBG_VERSION_MINOR_STR "." CXXDBG_VERSION_PATCHLEVEL_STR;

/// Version name
const char * const version_name = CXXDBG_VERSION_NAME;


}


