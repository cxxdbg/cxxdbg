# cxxdbg

**cxxdbg** is a standalone GUI debugger for C++, built on top of LLDB,
with advanced features for debugging complex programs written in
modern C++. Its goal is to improve the experience of debugging C++
programs. Linux is the primary target, but macOS and
Windows are also supported.

## Features

- Advanced stepping functions: stepping through standard functional
  objects (`std::function`, `std::bind` result) and user-defined
  functional objects matched by regular expressions, directly to the
  call target.

- Custom [`dbgfmt`](libs/dbgfmt/README.md) formatting framework
  implemented from scratch, with advanced support for the libstdc++
  and libc++ standard libraries. It supports the following types:
  - **Containers**: `array`, `vector`, `deque`, `list`, `forward_list`,
    `stack`, `queue`, unordered (hash table) containers, ordered (tree-based)
    containers, `bitset`, `valarray`.
  - **Strings**: `string`, `string_view`.
  - **Smart pointers**: `shared_ptr`,     `unique_ptr`, `auto_ptr`.
  - **Functional types**: `function`, `bind` results, argument placeholders,
    `reference_wrapper`.
  - **Utility types**: `tuple`, `optional`, `any`, `variant`, `complex`.
  - **`chrono`**: durations and time points.
  - **`regex`**.

  In addition to formatting values and types, it extracts the source
  code locations of types and values, which makes it possible to
  navigate to the source location of the target of a standard
  functional object, for example a `std::function`.

- Configurable stack trace filtering that hides frames of no
  interest, for example frames of standard or user-defined functional
  objects.

- Custom function name parsing that can handle complex template parameters.
  It enables correct detection of functions to step through or skip, and
  correct detection of stack frames to hide.

- A [Debug Adapter Protocol](https://microsoft.github.io/debug-adapter-protocol/)
  server (`cxxdbg-dap-server`), for use with editors and IDEs that
  implement the protocol.

- Customizable, dockable GUI built with Qt.

## Components

[`llvm-project`](https://github.com/cxxdbg/llvm-project), a fork of
LLVM/LLDB, is included in this repository as a git submodule.

Libraries developed for cxxdbg, maintained in separate repositories
and included in this repository as git submodules in `libs/`:

- [`dbgfmt`](libs/dbgfmt/README.md) — C++ debugger value/type
  formatting framework.
- [`cxxtn`](libs/cxxtn/README.md) — helper library for parsing C++
  type and function names.
- [`cxxdap`](libs/cxxdap/README.md) — Debug Adapter Protocol (DAP)
  library.

## External Dependencies

Third-party libraries included in this repository as git submodules
in `thirdparty/`:

- [`json`](https://github.com/nlohmann/json) — nlohmann/json,
  header-only.
- [`range-v3`](https://github.com/ericniebler/range-v3) — used
  instead of `std::ranges` when `CXXDBG_USE_RANGE_V3` is set.
- [`cm`](https://github.com/cxx-lang-tools/cm) — code model library,
  used in the dbgfmt library for unit tests.
- [`tmvc`](https://github.com/cxx-ui/tmvc) — text MVC library used
  for Qt text/document views in the GUI.

## Building

Building release with all dependencies and system Qt on Linux

```sh
mkdir build-dir
cd build-dir
cmake <source-dir> -DCMAKE_BUILD_TYPE=Release \
                   -DCMAKE_INSTALL_PREFIX=<prefix>
cmake --build .
cmake --build . --target install-cxxdbg
```

Building release with installed Qt
```sh
mkdir build-dir
cd build-dir
cmake <source-dir> -DCMAKE_BUILD_TYPE=Release \
                   -DCMAKE_INSTALL_PREFIX=<prefix> \
                   -DCMAKE_PREFIX_PATH=<qt-path-with-target-suffix>
cmake --build .
cmake --build . --target install-cxxdbg
```

CMake options:
- `CXXDBG_DEPLOYMENT_TYPE` — `Development` (default) or `Release`.
  Determines the default values of the options below (whether LLVM
  and other dependencies are built from source or taken from
  system/external installations, static vs. shared linking, etc).
- `CXXDBG_BUILD_DEPS` — build ncurses, libxml2, and other system dependencies
  from source instead of using system packages. Default is `ON`
  for `Release` deployment mode and `OFF` for `Development`.
- `CXXDBG_BUILD_LLVM` — build LLVM/LLDB from the
  `llvm-project` submodule instead of using prebuilt LLVM/LLDB
  specified via `LLVM_ROOT`.
- `LLVM_ROOT` — path to prebuilt LLVM/LLDB fork that will be used
  when `CXXDBG_BUILD_LLVM=OFF`
- `CXXDBG_USE_QT6` (default `ON`) — selects Qt6 instead of Qt5

### Building LLVM/LLDB in separate directory

This repo supports separate build of LLVM/LLDB. That may be useful
for development because it reduces build time and decreases size of
debugging information.

To build LLVM/LLDB in separate build dir:
```
mkdir llvm-build-dir
cd llvm-build-dir
cmake <source-dir>/build/cxxdbg-llvm -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

After build is done, the `llvm-build-dir/llvm` directory can be passed
to cxxdbg configure using the `LLVM_ROOT` cmake parameter for debug build:
```
mkdir build-dir
cd build-dir
cmake <source-dir> -DLLVM_ROOT=<llvm-build-dir>/llvm
cmake --build .
cmake --build . --target install-cxxdbg
```

## Testing

Tests are written with [Boost.Test](https://www.boost.org/doc/libs/release/libs/test/)
and registered with CTest. Each module with tests has a `test/`
subdirectory that builds a `<module>test` executable (for example,
`cxxdbg/util/test` produces `utiltest`). Run the full suite with `ctest`
from the build directory.

## License

cxxdbg is distributed under the [Apache License 2.0](LICENSE).
