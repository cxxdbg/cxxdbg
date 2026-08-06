// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file target_base.hpp
/// Contains definition of target_base class.

#pragma once

#include "code_breakpoint.hpp"
#include "instruction.hpp"
#include "function_debug_object_impl.hpp"
#include "global_variable.hpp"
#include "lldb_memory_storage.hpp"
#include "log.hpp"
#include "module.hpp"
#include "signals.hpp"
#include "symbol_impl.hpp"
#include "thread.hpp"
#include "type_impl.hpp"
#include "watchpoint.hpp"
#include "dbgfmt/data/endianness.hpp"
#include "dbgfmt/backend/type_context_impl.hpp"
#include "dbgfmt/backend/value_context_impl.hpp"

#include <lldb/API/SBTarget.h>
#include <lldb/lldb-forward.h>

#include <ranges.hpp>
#include <map>
#include <variant>
#include <vector>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/signals2/signal.hpp>


namespace dbgfmt {
    class value_impl;
}


namespace cxxdbg::dbg {

namespace core {


class code_breakpoint;
class code_model;
class debugger_base;
class stack_frame;
class varaible;
class watchpoint;


/// Base class for the target class. Contains LLDB dependent code
class target_base: public dbgfmt::backend::value_context_impl, public dbgfmt::backend::type_context_impl {
    friend class code_breakpoint;
    friend class code_position;
    friend class stack_frame;
    friend class thread_base;
    friend class variable;
    friend class watchpoint;

    /// Type of connection to a signal
    typedef boost::signals2::connection connection;

public:
    /// Target state
    enum class state_t {
        loaded,         ///< Target is loaded, but not started
        launching,      ///< Target is launching
        running,        ///< Target is running
        stopped,        ///< Target is stopped (interrupted)
    };

    /// Type of scoped connection to a signal
    typedef boost::signals2::scoped_connection scoped_connection;

    /// Type of state changed signal function
    typedef void state_changed_func(state_t);

    /// Type of state changed signal handler
    typedef std::function<state_changed_func> state_changed_handler;

    /// Returns name of state
    static std::string state_name(state_t st);

    /// Constructs target by loading executable with specified path
    target_base(debugger_base & dbg, const std::filesystem::path & exe_path);

    /// Consturcts target by attaching to process with specified pid
    target_base(debugger_base & dbg, unsigned long pid);

    /// Constructs target by attaching to process with specified name
    target_base(debugger_base & dbg, const std::string & name);

    /// Constructs dummy target
    target_base(debugger_base & dbg);

    /// Destructor, closes target
    ~target_base();

    /// Returns pointer to LLDB target
    lldb::SBTarget lldb_targ() const;

    /// Returns target pointer byte size
    size_t ptr_byte_size() const;


    //////////////////////////////////////////////////
    // Modules

    /// Type of iterator over modules
    class const_module_iterator;

    /// Returns number of modules in target
    std::size_t modules_size() const;

    /// Returns module at specified index
    module module_at(std::size_t i) const;

    /// Returns iterator pointing to the first module
    const_module_iterator modules_begin() const;

    /// Returns iterator pointing to the one past the last module
    const_module_iterator modules_end() const;

    /// Reindexes target types/symbols
    void reindex();

    /// Finds function with specified name. Returns code position.
    code_position find_func(const std::string & name);

    /// Finds type in specified lldb target
    lldb::SBTypeList find_types(const lldb::SBTarget & lldb_targ, const std::string & name);

    /// Modules loaded signal
    signal<void (const std::vector<module> &)> modules_loaded;

    /// Modules unloaded signal
    signal<void (const std::vector<module> &)> modules_unloaded;


    //////////////////////////////////////////////////
    // Value context implementation for formatting framework

    /// Returns pointer to implementation of value type
    /// with specified type name
    type_impl * find_type(const std::string & nm) override;

    /// Returns builtin type of specified kind
    type_impl * bt_type(dbgfmt::backend::builtin_type_impl::kind_t kind) override;

    /// Returns reference to memory storage
    dbgfmt::data::storage & memory() override { return mem_; }

    /// Returns data endianness for memory
    dbgfmt::data::endianness endianness() const override;

    /// Tries detect dynamic value type at specified address
    const type_impl * dynamic_type_at_addr(uint64_t & addr, const dbgfmt::backend::type_impl * static_type) override;

    /// Returns objevirtualct located at specified address or nullptr if object not found
    dbgfmt::backend::debug_object_impl * obj_at_addr(uint64_t addr) override;

    /// Returns symbol name located at specified address
    dbgfmt::backend::symbol_impl * sym_at_addr(uint64_t addr) override;

    /// Gets or cretes pointer to specified type
    type_impl * get_or_create_ptr_type(const type_impl * type);

    /// Gets or creates type for specified lldb type
    type_impl * get_or_create_type(const lldb::SBType & tp);


    //////////////////////////////////////////////////
    // State changing

    /// Returns target state
    state_t state() const;

    /// Returns last exit code for target in loaded state
    int exit_code() const;

    /// Returns signal number for target in stopped state
    int signal_num() const;

    /// Returns active breakpoint number for target in stopped state
    breakpoint_num active_bp_num() const;

    /// Returns active watchpoint number for target in stopped state
    breakpoint_num watchpoint_num() const;

    /// Launches target with specified working directory,
    /// command arguments, and result handler. May be called only in loaded
    /// state. After calling, the state of target becomes launching.
    /// Returns id of launched process
    unsigned long launch(const std::filesystem::path & work_dir,
                         const std::vector<std::string> & args,
                         const std::map<std::string, std::string> & env = {});

    /// Attaches to process with specified process id or name. May be called
    /// only in loaded state. After calling the state of target becomes launching.
    void attach(const std::variant<unsigned long, std::string> & targ);

    /// Continues execution of target. May be called in stopped state.
    /// After calling, the state of target becomes running.
    void run();

    /// Starts stopping target process. May be called only in running
    /// or stopped state. After calling, the state of target does not change
    /// before actual stopping of target.
    void stop();

    /// Termintes target process. May be called only in stopped
    /// state. After calling, the state of target becomes loaded.
    void terminate();

    /// Detaches from target process. May be called only in stopped
    /// state. After calling, the state of target becomes loaded.
    void detach();

    /// Performs "step into" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_into(unsigned long thread_id,
                   bool avoid_nodebug,
                   const std::string & target,
                   const std::string & avoid_regex,
                   const std::string & step_through_regex);

    /// Performs "step over" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_over(unsigned long thread_id,
                   const std::string & step_through_regex);

    /// Performs "step out" action on target. Can be called only in
    /// 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_out(unsigned long thread_id,
                  bool avoid_nodebug,
                  const std::string & step_through_regex);

    /// Performs "step instruction into" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_into(unsigned long thread_id);

    /// Performs "step instruction over" action on target. Can be called
    /// only in 'stopped' state. After calling, the state of target
    /// becomes 'running'.
    void step_inst_over(unsigned long thread_id);

    /// Performs "step until" action on target. Can be called only
    /// in 'stopped' state. After calling, the state of target becomes
    /// 'running'.
    void step_until(unsigned long thread_id, unsigned int line);

    /// Connects to state changed signal
    connection connect_state_changed(const state_changed_handler & handl);

    /// Returns ID of target process. May be called only in
    /// 'running' or 'stopped' state.
    unsigned long pid() const;


    //////////////////////////////////////////////////
    // Threads

    /// Returns number of threads in target
    std::size_t threads_size() const;

    /// Returns thread with specified index
    thread thread_at(std::size_t i);

    /// Returns index of current thread
    std::size_t current_thread_index();

    /// Sets index of current thread
    /// TODO: refactor and move current thread/frame logic to appcore library
    void set_current_thread_index(std::size_t index);

    /// Sets index of current stack frame
    /// TODO: refactor and move current thread/frame logic to appcore library
    void set_current_stack_frame_index(std::size_t index);


    //////////////////////////////////////////////////
    // Variables

    // TODO: merge global and local variables

    /// Gets or creates global variable for specified lldb variable and address
    global_variable * get_or_create_global_var(lldb_private::Variable * var, uint64_t addr);

    /// Gets or creates variable for specified lldv variable
    variable * get_or_create_var(const lldb::VariableSP & var);


    //////////////////////////////////////////////////
    // Breakpoints

    /// Returns range of breakpoints
    auto breakpoints() {
        auto fn = [](auto && bp_map_val) { return bp_map_val.second.get(); };
        return breakpoints_ | std::ranges::views::transform(fn);
    }

    /// Returns const range of breakpoints
    auto breakpoints() const {
        auto fn = [](auto && bp_map_val) -> const breakpoint* {
            return bp_map_val.second.get();
        };
        return breakpoints_ | std::ranges::views::transform(fn);
    }

    /// Returns pointer to breakpoint with specified id
    breakpoint * get_breakpoint_by_id(const breakpoint::id_t & id);

    /// Returns pointer to const breakpoint with specified id
    const breakpoint * get_breakpoint_by_id(const breakpoint::id_t & id) const;

    /// Removes breakpoint
    void remove_breakpoint(breakpoint * bp);

    /// Adds breakpoint with specified source position, returns pointer
    /// to breakpoint
    code_breakpoint * add_breakpoint(const core::source_position & pos);

    /// Adds breakpoint at function with specified name, returns pointer
    /// to breakpoint
    code_breakpoint * add_breakpoint(const std::string & func_name);

    /// Adds breakpoint at specified address, returns pointer
    /// to breakpoint
    code_breakpoint * add_breakpoint(std::uint64_t addr);

    /// Adds watchpoint for variable with specified name. Throws
    /// exception if watchpoint can't be set
    watchpoint * add_var_watchpoint(const std::string & var_name,
                                    bool read,
                                    bool write,
                                    size_t size);

    /// Adds watchpoint for address specified by expression. Throws
    /// exception if watchpoint can't be set
    watchpoint * add_expr_watchpoint(const std::string & expr,
                                     bool read,
                                     bool write,
                                     size_t size);


    //////////////////////////////////////////////////
    // Input / output

    /// Stdout data received signal
    CXXDBG_DEFINE_SIGNALX(stdout_received, void(const std::string &))

    /// Stderr data received signal
    CXXDBG_DEFINE_SIGNALX(stderr_received, void(const std::string &))

    /// Sends data to stdin of process being debugged
    void send_stdin(const std::string & data);


    //////////////////////////////////////////////////
    // Disassembler

    /// Disassembles instructions from specified address
    std::vector<instruction> disassemble(uint64_t addr, int64_t inst_offset, uint64_t inst_count);


    //////////////////////////////////////////////////
    // Memory

    /// Reads memory from target. Returns vector of bytes. Returned vector may have size
    /// less than the size parameter if not all memory block can be read.
    std::vector<char> read_memory(uint64_t addr, uint64_t size);


    /// Returns vector of all call targets in source position for
    /// specified code position
    std::vector<std::string> call_targets(const code_position & pos) const;


    /// Returns reference to code model for target
    const code_model & cm() const;

protected:
    /// Returns LLDB thread object with specified index
    lldb::SBThread lldb_thread_at(std::size_t i);

private:
    /// Compares pointers to two lldb function objects
    struct compare_lldb_func {
    public:
        bool operator()(const lldb::SBFunction & f1, const lldb::SBFunction & f2) const;
    };

    /// Compares pointers two lldb symbols by address
    struct compare_lldb_sym {
    public:
        compare_lldb_sym(const lldb::SBTarget & t):
            targ_{t} {}

        bool operator()(const lldb::SBSymbol & s1, const lldb::SBSymbol & s2) const;

    private:
        lldb::SBTarget targ_;
    };


    /// Constructor, makes target with specified reference to debugger and LLDB target
    target_base(debugger_base & dbg, const lldb::SBTarget & lt);

    /// Checks that debugging of target is possible. Throws
    /// exception if not
    void check_can_debug() const;

    /// Handles stdout event
    void handle_stdout();

    /// Handles stderr event
    void handle_stderr();

    /// Finds local or global variable with specified name
    lldb::SBValue find_var(const std::string & name);

    /// Creates new breakpoint ID
    int create_bp_id() {
        return next_bp_id_++;
    }

    /// Returns frame object for current selected frame
    /// TODO: refactor find_var and remove this function.
    virtual stack_frame_base selected_frame_base() = 0;


    /// Type of vector of shared pointers to breakpoint
    using breakpoint_map = std::map<breakpoint_id, breakpoint_sp>;

    debugger_base & dbg_;               ///< Reference to debugger
    lldb::SBTarget targ_;               ///< LLDB target
    breakpoint_map breakpoints_;        ///< Map of breakpoints in target
    int next_bp_id_ = 0;                ///< Next breakpoint ID

    /// Map for lldb types
    std::map<lldb::SBType, type_impl_sp> types_;

    /// Map for find_type results
    std::map<std::string, type_impl*> find_types_;

    /// Map for pointer types
    std::map<const type_impl *, type_impl*> ptr_types_;

    /// Map of global variables
    std::map<lldb_private::Variable*, std::unique_ptr<global_variable>> glob_vars_;

    /// Map of variables from LLDB value ID
    std::map<lldb::VariableSP, std::unique_ptr<variable>> vars_;

    /// Map of debug objects for functions
    std::map<lldb::SBFunction,
             std::unique_ptr<function_debug_object_impl>,
             compare_lldb_func
    > func_objects_;

    /// Map of debug objects for symbols
    std::map<lldb::SBSymbol, std::unique_ptr<symbol_impl>, compare_lldb_sym> sym_objects_;

    /// LLDB target memory storage
    lldb_memory_storage mem_;

    /// State changed signal
    boost::signals2::signal<state_changed_func> state_changed_signal_;

    /// Connection to lldb event signal
    scoped_connection lldb_event_con_;
};


class target_base::const_module_iterator: public boost::iterator_adaptor <
    const_module_iterator,
    std::size_t,
    module,
    boost::random_access_traversal_tag,
    module,
    std::size_t
> {
public:
    /// Constructor, makes iterator with specified reference to target
    /// and module index
    const_module_iterator(const target_base & targ, std::size_t i);

    /// Dereferences iterator
    reference dereference() const;
private:
    const target_base & targ_;          ///< Reference to target
};



} }
