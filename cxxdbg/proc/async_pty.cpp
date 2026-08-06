// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_pty.cpp
/// Contains implementation of the async_pty class.

#include "async_pty.hpp"
#include <termios.h>


namespace cxxdbg::proc {


static std::error_code get_last_error() {
    return std::error_code{errno, std::system_category()};
}


static void throw_error [[noreturn]] (const std::error_code & ec, const char * msg) {
    throw ::boost::process::process_error{ec, msg};
}


async_pty::async_pty(async_pty::io_context & ctx):
pty_master_{ctx} {
    // creating PTY
    int master_fd = ::posix_openpt(O_RDWR | O_CLOEXEC | O_NOCTTY | O_NONBLOCK);
    if (master_fd < 0) {
        throw_error(get_last_error(), "posix_openpt failed");
    }

    // granting access to slave PTY
    if (::grantpt(master_fd) < 0) {
        auto ec = get_last_error();
        ::close(master_fd);
        throw_error(ec, "grantpt failed");
    }

    // unlocking terminal
    if (::unlockpt(master_fd) < 0) {
        auto ec = get_last_error();
        ::close(master_fd);
        throw_error(ec, "unlockpt failed");
    }

    pty_master_.assign(master_fd);
}


async_pty::async_pty(async_pty && pty):
pty_master_{std::move(pty.pty_master_)} {
}


async_pty::~async_pty() {
    close();
}


async_pty & async_pty::operator=(async_pty && pty) {
    pty_master_ = std::move(pty.pty_master_);
    return *this;
}


int async_pty::open_slave() {
    auto pty_name = ::ptsname(pty_master_.native_handle());
    if (pty_name == nullptr) {
        throw_error(get_last_error(), "ptsname failed");
    }

    auto fd = ::open(pty_name, O_RDWR | O_CLOEXEC | O_NOCTTY);
    if (fd < 0) {
        throw_error(get_last_error(), "ptsname failed");
    }

//    // set raw mode on slave
//    termios sett;
//    ::tcgetattr(fd, &sett);
//    ::cfmakeraw(&sett);
//    ::tcsetattr(fd, TCSANOW, &sett);

    return fd;
}


}
