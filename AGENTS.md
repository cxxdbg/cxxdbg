# cxxdbg

`cxxdbg` is a Qt-based standalone GUI debugger for C/C++, built on top of
LLVM/LLDB. It embeds LLDB's `SBDebugger` API directly (via `cxxdbg/llvm`)
rather than shelling out to `lldb`, and also ships a DAP (Debug Adapter
Protocol) server mode and a standalone LLDB-compatible CLI frontend.

## Getting the code

This repo vendors its third-party and in-house dependencies as git
submodules (see [Vendored libraries](#vendored-libraries-thirdparty-libs)
below), so a fresh clone needs them initialized before `cmake` can
configure — `thirdparty/json`, `libs/cxxdap`, etc. will otherwise be empty
directories and configuration will fail:

```sh
git clone --recursive git@github.com:cxxdbg/cxxdbg.git
# or, if already cloned non-recursively:
git submodule update --init --recursive
```

`llvm-project` is one of these submodules and is a large clone; see
`CXXDBG_BUILD_LLVM`/`LLDB_ROOT` below for avoiding a from-source LLVM/LLDB
build on top of it.

## Building

Prerequisites: a C++20 compiler, CMake, Boost >= 1.74
([build/cmake/boost.cmake](build/cmake/boost.cmake)), and Qt5 or Qt6 for
the GUI (see `CXXDBG_USE_QT6` below).

```sh
mkdir build
cd build
cmake ..
cmake --build .
ctest
```

Key CMake options (all defined in the root `CMakeLists.txt`):

- `CXXDBG_DEPLOYMENT_TYPE` — `Development` (default) or `Release`. Controls
  a large set of other option defaults (whether LLVM/LLDB/deps are built
  from source vs. using system/external installs, static vs. shared
  linking, etc).
- `CXXDBG_BUILD_DEPS` — build ncurses/libxml2/etc from source instead of
  using system packages.
- `CXXDBG_BUILD_LLVM` / `CXXDBG_BUILD_LLDB` — build LLVM/LLDB from the
  `llvm-project` submodule instead of using an external prebuilt LLDB
  (`LLDB_ROOT`).
- `CXXDBG_USE_QT6` (default `ON`) — Qt6 vs Qt5. Pass `-DQt5_DIR=...` or
  `-DQt6_DIR=...` / put Qt on `CMAKE_PREFIX_PATH` as needed.
- `CXXDBG_ENABLE_CLI` — build the standalone `cxxdbg-cli` executable.
- `CXXDBG_ENABLE_SHARED` — build shared libraries instead of static.

A full local build compiles LLVM/LLDB from the submodule and is
heavyweight; for fast iteration on `cxxdbg/*` code alone, point
`LLDB_ROOT` at an already-built LLDB and leave
`CXXDBG_BUILD_LLVM`/`CXXDBG_BUILD_LLDB` off (the `Development` deployment
type does this by default).

## Testing

Tests use Boost.Test (`BOOST_TEST_MODULE`, `#include
<boost/test/included/unit_test.hpp>`) and are registered with CTest. Each
module with tests has its own `test/` subdirectory and `test/CMakeLists.txt`
building a `<module>test` executable (e.g. `cxxdbg/util/test` →
`utiltest`, `cxxdbg/dbg/test` → `dbgtest`).

`cxxdbg/mock` provides a small standalone mocking framework used across
these test suites.

## Layout

Repository root:

- `cxxdbg/` — all first-party source (see breakdown below).
- `thirdparty/`, `libs/` — vendored git submodules, see
  [Vendored libraries](#vendored-libraries-thirdparty-libs) below.
- `llvm-project/` — LLVM/LLDB submodule (`cxxdbg/llvm-project` fork), used
  when building LLVM/LLDB from source (`CXXDBG_BUILD_LLVM`/`CXXDBG_BUILD_LLDB`).
- `resources/` — app icon/toolbar images and bundled third-party license
  texts, see below.
- `build/` — not the build output directory itself (that's whatever
  out-of-tree directory you run `cmake` from); holds the CMake toolchain,
  shared `.cmake` modules, and the from-source dependency build
  (`build/cxxdbg-deps`) used when `CXXDBG_BUILD_DEPS` is on.

The codebase is organized as a layered set of CMake libraries under
`cxxdbg/` (see [cxxdbg/CMakeLists.txt](cxxdbg/CMakeLists.txt) for the full
subdirectory list):

- `mock` — standalone mocking framework used by the test suites.
- `util` — general-purpose utility library.
- `async` — async execution queue utilities.
- `proc` — process launching/monitoring library.
- `log` — logging library.
- `llvm` — wraps the LLDB driver code and bundles all LLVM/LLDB static
  libraries with hidden visibility; the embedding point used by both
  `cxxdbg-cli` and the GUI.
- `app` — the app/document layer: documents, tree/list view models,
  settings storage. Platform-agnostic, no LLDB dependency.
- `gui` — the Qt6 GUI (`cxxdbg` executable). `MainWindow`/`DebugUI` wire up
  menus and actions; dock widgets (breakpoints, call stack, locals,
  registers, threads, watches, source tree, filesystem, terminal, log)
  live alongside launch/attach/breakpoint-editing/settings dialogs. Links
  `dbg/appcore` directly — this is where the app/document layer meets the
  LLDB-backed debugger implementation.
- `exec_stub` — library for exchanging data between the exec stub and
  `cxxdbg`, plus the `cxxdbg-exec-stub` helper executable.
- `cli` — command-line interface library (`cxxdbg-dbg-cli`), linked in via
  `app`.
- `dbg` — the debugger domain model (breakpoints, threads, stack frames,
  watches, source trees, platforms, etc), independent of any particular
  UI:
  - `dbg/core` — the LLDB-backed implementation of the domain model
    (`cxxdbg-dbg-corellvm`, an LLVM-dependent object library, plus
    `cxxdbg-dbg-core` on top of it).
  - `dbg/appcore` — bridges `dbg/core` into the `app` layer's tree/list
    view model abstractions (locals, watches, registers, custom watches).
  - `dbg/dapsrv` — DAP server built on `dbg/core` + the vendored `cxxdap`
    library (VSCode-style debug-adapter mode); produces the
    `cxxdbg-dap-server` executable.
- `cxxdbg-cli` — standalone LLDB-compatible CLI frontend, bypassing the
  GUI and `dbg/appcore` layer, linking `dbg/core` + `llvm` directly.

`resources/images` and `resources/license` hold the app icon/toolbar
images and bundled third-party license texts; both stage their output
into `share/cxxdbg/{images,license}` under the build/install tree, which
the running app reads via `applicationDirPath() + "/../share/cxxdbg/..."`.

## Vendored libraries (`thirdparty/`, `libs/`)

Git submodules vendored under `thirdparty/`, added via
[thirdparty/CMakeLists.txt](thirdparty/CMakeLists.txt):

- `json` — nlohmann/json, header-only.
- `range-v3` — used when `CXXDBG_USE_RANGE_V3` is set instead of
  `std::ranges`.
- `cm` — code-model library: the type/value model used by `dbgfmt`'s
  bundled backend and its test suite.
- `tmvc` — text MVC library backing the Qt text/document views used
  throughout `cxxdbg/gui`.

Git submodules for in-house libraries developed alongside cxxdbg, split
into their own repositories, vendored under `libs/` and added via
[libs/CMakeLists.txt](libs/CMakeLists.txt):

- `cxxdap` — Debug Adapter Protocol (DAP) library.
- `cxxtn` — helper library for parsing C++ type and function names.
- `dbgfmt` — C++ debugger value/type formatting framework (pretty-printing
  of variables/expressions for the STL and other types); see
  [libs/dbgfmt/README.md](libs/dbgfmt/README.md).

## CMake conventions

- Always reference variables as `"${VAR}"` (quoted) in conditionals and
  comparisons, even where CMake would tolerate the bare form.
- Split multi-condition `if(...)` expressions across multiple lines, one
  condition per line, instead of one long line.

## Include order

`#include` directives are grouped into the following categories, in
this order:

1. Includes from the current directory/module.
2. Includes from subdirectories of the current module.
3. Includes from other directories/modules in this project.
4. Third-party library includes.
5. System includes.

Includes are not separated by blank lines, except:

- A blank line goes before a category's includes when that category has
  more than 5 includes, to visually set off the large group.
- A blank line always goes before an `#ifdef`/`#if` block of includes
  (e.g. platform-specific includes).

For example, in `cxxdbg/app/version.hpp`:

```cpp
#include "version_config.hpp"
#include <string>
```
