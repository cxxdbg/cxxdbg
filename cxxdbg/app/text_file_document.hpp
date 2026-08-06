// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file text_file_document.hpp
/// Contains definition of the text_file_document class.

#pragma once

#include "file_document.hpp"
#include "text_document.hpp"
#include <fstream>


namespace cxxdbg {


/// Represents text file document
class text_file_document: virtual public file_document,
                          virtual public text_document {
};


}


