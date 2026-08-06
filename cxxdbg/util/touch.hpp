// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file touch.hpp
/// Contains declaration of the touch function

#pragma once

#include <filesystem>


namespace cxxdbg::util {


/// Cretes new empty file or changes modification time of existing file
void touch(const std::filesystem::path & p);


}


