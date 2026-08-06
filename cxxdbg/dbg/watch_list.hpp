// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list.hpp
/// Contains definition of watch_list class.

#pragma once

#include <list>
#include <map>
#include <queue>
#include <set>

#include "source_position_info.hpp"
#include "cxxdbg/app/signals.hpp"
#include "cxxdbg/app/tree_view_model.hpp"
#include "dbgfmt/format_options.hpp"


namespace dbgfmt {
    class format_options;
}


namespace cxxdbg::dbg {

class watch_list_impl;
class watch_list_impl_tree_info;
class watch_list_impl_tree_info_node;


/// \class watch_list_tree_node
/// A node in values tree in watch list
class watch_list_tree_node {
    friend class watch_list;

public:
    /// Type of node ID
    using node_id = unsigned long;

    /// Invalid node ID
    static constexpr node_id undef_node_id = ULONG_MAX;

    typedef std::shared_ptr<watch_list_tree_node> ptr;

    /// Constructor, makes tree node with specified id, name, value, and type,
    /// and initializes vector of empty childs with specified size
    explicit watch_list_tree_node(watch_list_tree_node * parent,
                                  node_id i = undef_node_id,
                                  const std::wstring & nm = {},
                                  const std::wstring & v = {},
                                  const std::wstring & t = {},
                                  std::size_t nchilds = 0,
                                  unsigned long fmt_time = ULONG_MAX);

    /// Destructor, destroys object
    ~watch_list_tree_node() = default;

    /// Returns pointer to parent node
    watch_list_tree_node * parent() {return parent_; }

    /// Returns const pointer to parent node
    const watch_list_tree_node * parent() const { return parent_; }

    /// Returns name field of tree node
    auto & name() const { return name_; }

    /// Sets name field
    void set_name(const std::wstring & nm) { name_ = nm; }

    /// Returns value field of tree node
    auto & value() const { return value_; }

    /// Sets value field
    void set_value(const std::wstring & v) { value_ = v; }

    /// Returns type field of tree node
    auto & type() const { return type_; }

    /// Sets type field
    void set_type(const std::wstring & t) { type_ = t; }

    /// Sets changed flag
    void set_marked(bool flag) { changed_flag_ = flag; }

    /// Returns changed flag
    bool is_marked() const { return changed_flag_; }

    /// Returns number of childs of tree node
    std::size_t childs_size() const { return childs_.size(); }

    /// Returns pointer to const child value with specified index
    const watch_list_tree_node * child_at(std::size_t index) const;

    /// Returns pointer to child value with specified index
    watch_list_tree_node * child_at(std::size_t index);

    /// Removes all child nodes and make n empty childs nodes
    void reset_childs(std::size_t n);

    /// Resets children size preserving data
    void reset_childs_preserve_raw_data(std::size_t n);

    /// Sets child at i-th place
    void set_child_at(std::size_t i, ptr node);

    /// Returns node expand state
    tree_view_expand_state expand_state() const { return expand_state_; }

    /// Sets node expand state
    void set_expand_state(tree_view_expand_state state) { expand_state_ = state; }

    /// Returns node definition position
    auto & pos() const { return pos_; }

    /// Returns definition position of object pointed by value
    auto & val_pos() const { return val_pos_; }

    /// Retursn value type definition position
    auto & type_pos() const { return type_pos_; }

    /// Sets node definition position
    void set_pos(const source_position_info & pos) { pos_ = pos; }

    /// Sets definition position of object pointed by value
    void set_val_pos(const source_position_info & pos) { val_pos_ = pos; }

    /// Sets value type definition position
    void set_type_pos(const source_position_info & pos) { type_pos_ = pos; }

    /// Returns root node pointer
    watch_list_tree_node * root();

    /// Returns const root node pointer
    const watch_list_tree_node * root() const;

    /// Returns node id or undef_node_id if node is not valid
    node_id id() const { return id_; }

    /// Returns format time for node in milliseconds
    unsigned long format_time() const { return fmt_time_; }

    /// Sets format time for node in milliseconds
    void set_format_time(unsigned long t) { fmt_time_ = t; }

private:
    /// Sets id for node
    void set_id(node_id i) { id_ = i; }

    /// Type shared pointer to node
    typedef std::shared_ptr<watch_list_tree_node> watch_list_tree_node_sp;

    /// Type of vector of tree nodes
    typedef std::vector<watch_list_tree_node_sp> watch_list_tree_node_vector;

    watch_list_tree_node * parent_;         ///< Parent node
    node_id id_;                            ///< Node id
    std::wstring name_;                     ///< Node name
    std::wstring value_;                    ///< Node value
    std::wstring type_;                     ///< Node type
    watch_list_tree_node_vector childs_;    ///< Vector of child nodes
    bool changed_flag_;                     ///< Changed flag, indicates that node was changed

    /// Node expand state
    tree_view_expand_state expand_state_ = tree_view_expand_state::collapsed;

    source_position_info pos_;      ///< Node definition position
    source_position_info val_pos_;  ///< Definition position of objects pointed by value
    source_position_info type_pos_; ///< Type definition position

    unsigned long fmt_time_;        ///< Format time
};

/// \class watch_list
/// Represents abstract watch list in debugger (locals, autos, custom watch).
class watch_list {
public:
    /// Watch tree node
    typedef watch_list_tree_node tree_node;

    /// Type of node id
    using node_id = tree_node::node_id;

    /// Undefined node id
    static constexpr auto undef_node_id = tree_node::undef_node_id;

    /// Type of const iterator over root tree nodes
    class const_root_node_iterator;

    /// Constructors watch list with specified pointer to watch list implementation
    explicit watch_list(watch_list_impl * impl = nullptr);

    /// Destructor, destroys object and all tree nodes
    virtual ~watch_list();

    /// Sets new pointer to implementation. Watch list will disconnect from all target
    /// and connect to the new one.
    virtual void set_impl(watch_list_impl * impl);

    /// Returns number of top level nodes
    std::size_t root_nodes_size() const;

    /// Returns const iterator pointing to the first root node
    const_root_node_iterator root_nodes_begin() const;

    /// Returns const iterator pointing to the one past last root node
    const_root_node_iterator root_nodes_end() const;

    /// Starts fetching child nodes for specified tree node from debugger
    void expand_node(const tree_node * node);

    /// Stops fetching child nodes for specified tree node and mark it as collapsed
    void collapse_node(const tree_node * node);

    /// Returns current format options
    auto & fmt_opts() const { return fmt_opts_; }

    /// Sets format options. Updates all nodes after receiving newly formatted tree
    void set_fmt_opts(const dbgfmt::format_options & opts);

    /// Root node added signal. The signal is emitted after root node has been added
    CXXDBG_DEFINE_SIGNAL(root_node_added, void (const tree_node*))

    /// Root node removed signal. The signal is emitted before root node is removed
    CXXDBG_DEFINE_SIGNAL(root_node_removed, void (const tree_node*))

    /// Node changed signal. The signal is emitted after node name/value/type has been changed
    CXXDBG_DEFINE_SIGNAL(node_changed, void (const tree_node*))

    /// Before added signal, emitted before adding new row
    CXXDBG_DEFINE_SIGNAL(before_added, void(tree_node*, std::size_t, std::size_t))

    /// After added signal, emitted after adding new row
    CXXDBG_DEFINE_SIGNAL(after_added, void(tree_node*, std::size_t, std::size_t))

    /// Before removed signal, emitted before removing row
    CXXDBG_DEFINE_SIGNAL(before_removed, void(tree_node*, std::size_t, std::size_t))

    /// After removed signal, emitted after removig row
    CXXDBG_DEFINE_SIGNAL(after_removed, void(tree_node*, std::size_t, std::size_t))

protected:
    /// Type of shared pointer to node
    typedef std::shared_ptr<tree_node> tree_node_sp;

    /// Adds new root node
    void add_root_node(const tree_node_sp & node);

    /// Removes root node. Returns shared pointer to node removed from watch
    tree_node_sp remove_root_node(const tree_node * node);

    /// Updates node values. Returns pair of changed flags: the first is
    /// for changed name/value/type, and the second one is for changed positions
    std::tuple<bool, bool> update_node_values(tree_node * node,
                                              const watch_list_impl_tree_info_node & node_info);

    /// Updates tree node
    virtual void update_node(tree_node * node,
                             const watch_list_impl_tree_info_node & node_info,
                             bool mark_changed);

    /// Returns node id
    static node_id get_node_id(const tree_node * node);

    /// Sets ID of root node
    void set_root_node_id(tree_node * node, node_id new_id);

    /// Called after complete watch tree has been updated in implementation. This happens
    /// after stepping, stopping at the breakpoint, changing current frame, etc.
    /// Another case is updating watch tree after changing format options
    void on_watch_tree_updated(const watch_list_impl_tree_info & tree, bool mark_changed_nodes);

    /// Resets childs in node
    void reset_childs(tree_node * node, const watch_list_impl_tree_info_node & node_info);

    /// Processes new node in tree info received from implementation
    virtual void process_new_node(const watch_list_impl_tree_info_node & node_info);

    /// Traverses the tree and makes visual expand state correspond to actual
    void update_tree_expand_state(tree_node * node);

    /// Clears all node values except of name and removes all childs
    void clear_node(const tree_node * node, bool clear_id);

    /// Sets new pointer to implementation. Disconnects and connects signals. Removes
    /// all root nodes if remove_roots flag is true
    void do_set_impl(watch_list_impl * impl, bool remove_roots);

private:
    /// Returns reference to implementation
    auto & impl() {
        assert(impl_ != nullptr && "implementation is null");
        return *impl_;
    }

    /// Returns true if pointer to implementation is not null
    bool has_impl() const { return impl_ != nullptr; }

    /// Connects to implementation signals
    void connect_impl();

    /// Disconnects from implementation signals
    void disconnect_impl();

    /// Traverses all child nodes recursively and
    /// removes them from the set of nodes being expanded or collapsed (exp_coll_nodes_).
    void remove_childs_from_exp_col(tree_node * node);

    /// Removes all childs of node. Traverses all child nodes recursively and
    /// removes them from the set of nodes being expanded or collapsed (exp_coll_nodes_).
    /// Sends before/after removed signals.
    void remove_childs(tree_node * node);

    /// Called after node expanding is complete in implementation
    void on_node_expand_complete(tree_node * node, const watch_list_impl_tree_info_node & node_info);

    /// Called after node collapsing is complete in implementation
    void on_node_collapse_compete(tree_node * node, const watch_list_impl_tree_info_node & node_info);

    /// Emits root node added signal and writes log
    void emit_root_node_added(const tree_node * node);

    /// Emits root node removed signal and writes log
    void emit_root_node_removed(const tree_node * node);

    /// Emits node changed signal and writes log
    void emit_node_changed(const tree_node * node);

    /// Emits before added signal and writes log
    void emit_before_added(tree_node * node, size_t first, size_t last);

    /// Emits after added signal and writes log
    void emit_after_added(tree_node * node, size_t first, size_t last);

    /// Emits before removed signal and writes log
    void emit_before_removed(tree_node * node, size_t first, size_t last);

    /// Emits after added signal and writes log
    void emit_after_removed(tree_node * node, size_t first, size_t last);

    /// Increases expand or collapse operation counter for node
    void inc_node_expand_collapse_ops(node_id id);

    /// Decreases expand or collapse operation counter for node if node
    /// found in map (counter is greater than zero). Returns true if
    /// node found in map, false overwise.
    bool dec_node_expand_collapse_ops(node_id id);

    /// Type of map from node id to tree node
    typedef std::map<node_id, tree_node*> tree_node_map;

    watch_list_impl * impl_;                        ///< Pointer to implementation
    tree_node_map root_nodes_map_;                  ///< Map of root nodes
    std::list<tree_node_sp> root_nodes_;            ///< List of root nodes

    /// Map of nodes being expanded or collapsed. Mapped number is the number of
    /// operations performed for node at the moment.
    std::map<node_id, unsigned int> exp_coll_nodes_;

    /// Connection to tree updated signal in implementation
    scoped_signal_connection watch_tree_updated_con_;

    /// Current format options
    dbgfmt::format_options fmt_opts_;
};


class watch_list::const_root_node_iterator: public boost::iterator_adaptor <
    const_root_node_iterator,
    std::list<tree_node_sp>::const_iterator,
    const tree_node *,
    boost::bidirectional_traversal_tag,
    const tree_node *
> {
public:
    /// Constructor, makes iterator with specified map iterator
    explicit const_root_node_iterator(const std::list<tree_node_sp>::const_iterator & it):
        iterator_adaptor_(it) {}

    /// Dereferences iterator
    reference dereference() const {
        return this->base()->get();
    }
};


}


