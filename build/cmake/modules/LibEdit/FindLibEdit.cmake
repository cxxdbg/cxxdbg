# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

set(LibEdit_FOUND TRUE)

set(LibEdit_LIBRARIES edit)

message(STATUS "FindLibEdit: using libedit built together with cxxdbg")

# We need export command for edit target because LLVM uses exports and requires
# that libedit is exported
export(TARGETS edit FILE libedit_export.cmake)
