// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file main.cpp
/// Main entry for CXXDBG DAP server

#include "log.hpp"
#include "server_impl.hpp"
#include <cxxdap/iostream_server.hpp>
#include <cxxdap/proxy_dumping_source.hpp>
#include <cxxdap/tcp_server.hpp>
#include "cxxdbg/app/version.hpp"
#include "cxxdbg/log/log_init.hpp"
#include <ranges.hpp>
#include <iostream>
#include <sstream>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/program_options.hpp>
#include "cxxdbg/boost_process/process.hpp"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif


namespace ba = boost::asio;
namespace bp = boost::process;
namespace fs = std::filesystem;
namespace po = boost::program_options;
namespace io = boost::iostreams;


/// Processes IO for ASIO context
static void process_io(ba::io_context & ctx) {
    CXXDBG_DAP_LOG_DEBUG << "starting processing IO";

    try {
        ctx.run();
    }
    catch(std::exception & err) {
        CXXDBG_DAP_LOG_ERROR << "exception while processing IO: " << err.what();
    }
    catch(...) {
        CXXDBG_DAP_LOG_ERROR << "unknown exception while processing IO";
    }

    CXXDBG_DAP_LOG_DEBUG << "finished processing IO";
}


std::vector<char> stdout_data;
std::vector<char> stdout_read_data(1024);

/// Reads data from stdout and redirects all to log
void start_read_stdout(bp::async_pipe & pipe) {
    ba::mutable_buffer buf{stdout_read_data.data(), stdout_read_data.size()};
    pipe.async_read_some(buf, [&pipe](const auto & ecode, size_t sz) {
        if (ecode || sz == 0) {
            // stdout pipe was closed, don't continue reading more data
            CXXDBG_DAP_LOG_DEBUG << "stdout pipe was closed";
            return;
        }

        // adding read data to stdout_data
        stdout_data.insert(stdout_data.end(), stdout_read_data.begin(), stdout_read_data.begin() + sz);

        // searching for eol symbols in stdout data and sending lines to log
        while (true) {
            auto it = std::find_if(stdout_data.begin(), stdout_data.end(), [](auto c) {
                return c == '\r' || c == '\n';
            });

            if (it == stdout_data.end()) {
                break;
            }

            CXXDBG_DAP_LOG_DEBUG << "stdout: " << std::string{stdout_data.begin(), it};

            if (*it == '\r') {
                ++it;
                if (it != stdout_data.end() && *it == '\n') {
                    ++it;
                }
            } else {
                ++it;
            }

            stdout_data.erase(stdout_data.begin(), it);
        }

        // reading more data
        start_read_stdout(pipe);
    });
}


void stdout_thread_func(ba::io_context & ctx, bp::async_pipe & pipe) {
    start_read_stdout(pipe);
    ctx.run();
}


int main(int argc, char * argv[]) {
    try {
        po::options_description desc("Common options");
        desc.add_options()
            ("help,h", "Display help message and exit")
            ("version,v", "Display version information and exit")
            ("stdin-dump-file", po::value<fs::path>(), "Path to file for stdin full dump")
            ("host", po::value<std::string>(), "Host for listening TCP server (default is localhost if port is specified)")
            ("port,p", po::value<std::string>(), "Port for listening TCP server (default is 4711 if host is specified");

        desc.add(cxxdbg::log::log_options());

        po::variables_map vars;
        po::store(po::parse_command_line(argc, argv, desc), vars);
        vars.notify();

        std::string ver_str =
            std::string{"CXXDBG Debug Adapter Protocol server version "} + cxxdbg::version_display_str();

        // displaying help message if help option is specified
        if (vars.count("help")) {
            std::cerr << ver_str << "\n\n" << desc;
            return 1;
        }

        // displaying version
        if (vars.count("version")) {
            std::cerr << ver_str << "\n";
            return 1;
        }

#ifdef _WIN32
        auto home_dir_var_name = "USERPROFILE";
#else
        auto home_dir_var_name = "HOME";
#endif

        auto log_file_path = fs::path(getenv(home_dir_var_name)) / ".cxxdbg/cxxdbg-dap-server.log";

        if (vars.count("host") || vars.count("port")) {
            // TCP server mode

            // initializing log with default console logger
            cxxdbg::log::init(vars, true, log_file_path);

            CXXDBG_DAP_LOG_INFO << "Starting CXXDBG DAP in TCP server mode";

            const auto & host = vars.count("host") > 0 ? vars["host"].as<std::string>() : "localhost";
            auto port = vars.count("port") > 0 ? vars["port"].as<std::string>() : "4711";

            ba::io_context ctx;
            ba::ip::address addr;
            ba::ip::tcp::resolver resolver{ctx};
            boost::system::error_code ec;
            auto resolve_res = resolver.resolve(host, vars["port"].as<std::string>(), ec);
            if (ec) {
                // error resolving host
                std::ostringstream str;
                str << "Can't resolve host '" << host << "' and port '" << port << "': " << ec.message() << "\n";
                throw std::runtime_error{str.str()};
            }

            assert(!std::ranges::empty(resolve_res) && "range returned by resolve should not be empty");
            auto endpoint = *std::ranges::begin(resolve_res);

            // starting TCP server
            cxxdap::tcp_server srv{ctx, endpoint, [&ctx] {
                return std::make_unique<cxxdbg::dbg::dapsrv::server_impl>(ctx.get_executor());
            }};

            // processing IO
            process_io(ctx);

        } else {
            // initializing log with default file logger
            cxxdbg::log::init(vars, false, log_file_path);

            CXXDBG_DAP_LOG_INFO << "Starting CXXDBG DAP in stdio mode";

            // setting stdin/stdout to binary mode on Windows
#ifdef _WIN32
            _setmode(_fileno(stdin), _O_BINARY);
            _setmode(_fileno(stdout), _O_BINARY);
#endif

            ba::io_context ctx;

            // creating io context and pipe for stdout replacement
            ba::io_context stdout_pipe_ctx;
            bp::async_pipe stdout_pipe{stdout_pipe_ctx};

            // duplicating stdout fd and replacing it with pipe,
            // closing stderr and replacing it with pipe
            // duplicating stdin fd and closing original fd
#ifdef _WIN32
            int stdout_dup = ::_dup(_fileno(stdout));
            ::close(_fileno(stdout));
            ::close(_fileno(stderr));
            auto stdout_pipe_fd = _open_osfhandle(reinterpret_cast<intptr_t>(stdout_pipe.native_sink()), 0);
            ::_dup2(stdout_pipe_fd, _fileno(stdout));
            ::_dup2(stdout_pipe_fd, _fileno(stderr));

            int stdin_dup = ::_dup(_fileno(stdin));
            ::close(_fileno(stdin));
#else
            int stdout_dup = ::dup(STDOUT_FILENO);
            ::close(STDOUT_FILENO);
            ::close(STDERR_FILENO);
            ::dup2(stdout_pipe.native_sink(), STDOUT_FILENO);
            ::dup2(stdout_pipe.native_sink(), STDERR_FILENO);

            int stdin_dup = ::dup(STDIN_FILENO);
            ::close(STDIN_FILENO);
#endif

            // Some external tools misbehave if stdin file handle (zero) is closed, since a later
            // socket() call may then reuse that file number and confuse buggy error-checking code
            // in those tools. To work around this we call the socket function here that will
            // reuse the first (zero) file number.
            if (::socket(AF_INET, SOCK_STREAM, 0) == -1) {
                throw std::runtime_error("socket function failed");
            }

            // creating istream from stdin duplicate
            io::file_descriptor stdin_dup_dev{stdin_dup, io::never_close_handle};
            io::stream_buffer stdin_buf{stdin_dup_dev};
            std::istream stdin_istr{&stdin_buf};

            // creating ostream to stdout duplicate
            io::file_descriptor_sink stdout_dup_dev{stdout_dup, io::never_close_handle};
            io::stream_buffer stdout_buf{stdout_dup_dev};
            std::ostream stdout_ostr{&stdout_buf};
            cxxdap::ostream_sink snk{stdout_ostr};

            // starting stdio server
            ba::executor exec = ctx.get_executor();
            cxxdbg::dbg::dapsrv::server_impl impl{exec};

            std::unique_ptr<cxxdap::source> stdin_src = std::make_unique<cxxdap::istream_source>(stdin_istr, exec);
            std::unique_ptr<cxxdap::source> src;

            if (vars.count("stdin-dump-file") > 0) {
                src = std::make_unique<cxxdap::proxy_dumping_source>(*stdin_src, vars["stdin-dump-file"].as<fs::path>());
            } else {
                src = std::move(stdin_src);
            }

            cxxdap::server srv{impl, *src, snk};

            // listening for server input close and closing stdout pipe to exit read loop
            srv.input_closed.connect([&stdout_pipe] {
                stdout_pipe.close();
            });

            // starting server
            srv.start();

            // starting processing stdout pipe in separate thread. We must use
            // separate thread to avoid deadlocks on synchrnous writes to stdout
            // (see 761 for more details).
            std::thread stdout_io_thread{[&stdout_pipe_ctx, &stdout_pipe] {
                stdout_thread_func(stdout_pipe_ctx, stdout_pipe);
            }};

            // prcessing IO
            process_io(ctx);

            // waiting for stdout pipe thread termination
            stdout_io_thread.join();
        }
    }
    catch (std::exception & err) {
        CXXDBG_DAP_LOG_ERROR << err.what();
        return 2;
    }

    CXXDBG_DAP_LOG_INFO << "exit";
    return 0;
}
