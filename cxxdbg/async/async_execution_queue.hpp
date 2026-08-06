// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file async_exection_queue.hpp
/// Contains definition of async_execution_queue class and related classes.

#pragma once

#include "result.hpp"
#include "cxxdbg/async/event_queue.hpp"
#include <cassert>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>


namespace cxxdbg { namespace async {


/// \class command_base
/// Base class of all asynchornous exection commands
class command_base {
public:
    /// Destructor, destroys object
    virtual ~command_base() {}

    /// Executes command with specified reference to asynchronous event handler
    virtual void execute(event_queue & handler) const = 0;
};


/// \class command_executor
/// Command executor for specified type of result
template <typename ResType, typename CmdHandler>
struct command_executor {
    static void execute(result<ResType> & res, const CmdHandler & cmd) {
        res.set_value(cmd());
    }
};


/// command_executor specialization for void result
template <typename CmdHandler>
struct command_executor<void, CmdHandler> {
    static void execute(result<void> & res, const CmdHandler & cmd) {
        cmd();
        res.set_ok();
    }
};


/// \clas command_handler_executor
/// Handler executor for specified type of result and EH flag
template <typename ResHandler, typename ResType, bool ThrowEh>
struct command_handler_executor {
    static void execute(const result<ResType> & res, const ResHandler & handl) {
        handl(res);
    }
};


/// command_handler_executor specialization for EH and void result
template <typename ResHandler>
struct command_handler_executor<ResHandler, void, true> {
    static void execute(const result<void> & res, const ResHandler & handl) {
        // throwing exception if needed
        if (!res.is_ok()) {
            throw std::runtime_error(res.error());
        }

        handl();
    }
};


/// command_handler_executor specialization for EH
template <typename ResHandler, typename ResType>
struct command_handler_executor<ResHandler, ResType, true> {
    static void execute(const result<ResType> & res, const ResHandler & handl) {
        // throwing exception if needed
        if (!res.is_ok()) {
            throw std::runtime_error(res.error());
        }

        handl(res.value());
    }
};


/// command_handler_executor specialization for std::tuple + EH
template <typename ResHandler, typename ... TupleTypes>
struct command_handler_executor<ResHandler, std::tuple<TupleTypes...>, true> {
private:
    typedef std::tuple<TupleTypes...> tuple_type;

    template <std::size_t ... Indexes>
    static void execute_helper(const ResHandler & handl,
                               const tuple_type & res,
                               const std::index_sequence<Indexes...> &) {
        handl(std::get<Indexes>(res)...);
    }

public:
    static void execute(const result<tuple_type> & res, const ResHandler & handl) {
        // throwing exception if needed
        if (!res.is_ok()) {
            throw std::runtime_error(res.error());
        }

        typedef std::make_index_sequence<sizeof...(TupleTypes)> indexes;
        execute_helper(handl, res.value(), indexes());
    }
};


/// \class command
/// Command for asynchronous exection in async_exection_thread.
template <typename ResType, typename CmdHandler, typename ResHandler, bool ThrowEh>
class command: public command_base {
public:
    /// Type of result
    typedef result<ResType> result_type;

    /// Constructor, makes new command with specified command and result handler
    command(const CmdHandler & cmd, const ResHandler & handl):
        cmd_(cmd), res_handl_(handl) {}

    /// Destructor, destroys object
    virtual ~command() {}

    /// Executes command with specified reference to asynchronous event handler
    virtual void execute(event_queue & async_handler) const {

        result_type res{"error"};

        // execute command and handle exceptions
        try {
            command_executor<ResType, CmdHandler>::execute(res, cmd_);
        }
        catch(std::exception & ex) {
            res.set_error(ex.what());
        }

        // call result handler.
        // we can't capture 'this' here becase command object
        // may be destroyed before exection of result handler
        ResHandler rh = res_handl_;
        async_handler.post([rh, res]() {
            command_handler_executor<ResHandler, ResType, ThrowEh>::execute(res, rh);
        });
    }

private:
    CmdHandler cmd_;        ///< Command handler
    ResHandler res_handl_;  ///< Result handler
};


/// \class async_exection_queue
/// Wrapper on top of event_queue for async command execution and result handling
class execution_queue {
public:
    /// Type of shared pointer to async command
    typedef std::shared_ptr<command_base> command_ptr;

    /// Constructor, makes execution queue with specified exec thread event queue
    /// and result handling event queue
    execution_queue(event_queue & equeue, event_queue & hqueue);

    /// Destructor, destroys object
    ~execution_queue();

    // non copyable
    execution_queue(const execution_queue &) = delete;
    execution_queue & operator=(const execution_queue &) = delete;

    /// Adds command with specified command handler and result handler
    template <typename ResHandler, typename CmdHandler>
    void add_command(const ResHandler & res_handler, const CmdHandler & cmd_handler) {
        command<decltype(cmd_handler()), CmdHandler, ResHandler, false> cmd(cmd_handler, res_handler);
        exec_queue_.post([cmd, this] () {
            cmd.execute(handle_queue_);
        });
    }

    /// Adds command with specified command handler and result handler.
    /// Throws exception in callers thread in case of exception in async command handler
    template <typename ResHandler, typename CmdHandler>
    void add_command_eh(const ResHandler & res_handler, const CmdHandler & cmd_handler) {
        command<decltype(cmd_handler()), CmdHandler, ResHandler, true> cmd(cmd_handler, res_handler);
        exec_queue_.post([cmd, this] () {
            cmd.execute(handle_queue_);
        });
    }

    /// Returns reference to result handling queue
    event_queue & handle_queue() {
        return handle_queue_;
    }

private:
    event_queue & exec_queue_;          ///< Execution queue
    event_queue & handle_queue_;        ///< Result handling queue
};


} }


