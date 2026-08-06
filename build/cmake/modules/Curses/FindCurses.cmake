# Copyright (c) 2026, Alexandr Esilevich
#
# Licensed under the Apache License, Version 2.0.
# See accompanying file LICENSE for license information.
# SPDX-License-Identifier: Apache-2.0
#

set(Curses_FOUND TRUE)

set(CURSES_LIBRARIES ncurses)
set(PANEL_LIBRARIES panel)

# we don't need additional include directories, but lldb build scripts require
# that FindLibEdit sets the CURSES_INCLUDE_DIRS variable
set(CURSES_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/ncurses/gen/include")

message(STATUS "FindCurses: using ncurses built together with cxxdbg")

# We need export command for ncurses and panel targets because LLVM uses exports and requires
# that all dependent libraries have exports
export(TARGETS ncurses panel FILE ncurses_export.cmake)
