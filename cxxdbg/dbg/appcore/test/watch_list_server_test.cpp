// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file watch_list_server_test.cpp
/// Contains unit tests for watch_list_server class.

#include <iostream>
#include <list>

#include "fmt_result_tree.hpp"
#include "../watch_list_server.hpp"
#include "dbgfmt/backend/value_context_impl.hpp"
#include "cxxdbg/util/treewalk.hpp"

#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::test {


/// class test_tree_node used for creating test trees having id, name and type in each node
class test_tree_node {
public:
    /// Constructor
    test_tree_node(size_t id, std::wstring name, std::wstring type, std::vector<test_tree_node> childs = {}):
            id_{id}, name_(std::move(name)), type_(std::move(type)), childs_(std::move(childs)) {
    }

    /// Returns node id
    auto id() const { return id_; }

    /// Returns node name
    auto & name() const { return name_; }

    /// Returns node type
    auto & type() const { return type_; }

    /// Returns childs count
    auto childs_size() const { return childs_.size(); }

    /// Returns child at specified position
    auto & child_at(size_t idx) { return childs_[idx]; }

    /// Returns child at specified position
    auto & child_at(size_t idx) const { return childs_[idx]; }

private:
    size_t id_;                            ///< Node id
    std::wstring name_;                    ///< Node name
    std::wstring type_;                    ///< Node type
    std::vector<test_tree_node> childs_;   ///< Child nodes
};

}

namespace cxxdbg::dbg::util::tree {

/// partial specialization of comparator class for dbgfmt::named_fmt_result
/// because we cannot return dbgfmt::named_fmt_result based on fmt_result_node by reference, only by value
template <>
struct tree_comparator<dbgfmt::named_fmt_result, dbg::test::test_tree_node> {
    using L = dbgfmt::named_fmt_result;
    using R = dbg::test::test_tree_node;
    using compare_func = std::function<bool(L, const R &)>;
    using left_count_func = std::function<size_t (L)>;
    using left_get_func = std::function<L (L, size_t)>;
    using right_count_func = std::function<size_t (const R &)>;
    using right_get_func = std::function<const R& (const R &, size_t)>;

    tree_comparator(
            compare_func compare, left_count_func lc, left_get_func lg, right_count_func rc, right_get_func rg):
            compare_{compare}, lc_{lc}, lg_{lg}, rc_{rc}, rg_{rg} {
    }

    bool operator()(L l, const R & r) {
        if (! compare_(l, r))
            return false;

        size_t count = lc_(l);
        if (count != rc_(r))
            return false;

        for (size_t i = 0; i < count; ++i) {
            auto lchild = lg_(l, i);
            const auto & rchild = rg_(r, i);
            if (! (*this)(lchild, rchild))
                return false;
        }

        return true;
    }

private:
    compare_func compare_;
    left_count_func lc_;
    left_get_func lg_;
    right_count_func rc_;
    right_get_func rg_;
};


template <>
struct tree_walker<dbgfmt::named_fmt_result> {
public:
    typedef std::function<void(dbgfmt::named_fmt_result, size_t shift)> visit_func;
    typedef std::function<size_t(dbgfmt::named_fmt_result & )> count_func;
    typedef std::function<dbgfmt::named_fmt_result (dbgfmt::named_fmt_result &, size_t)> at_func;

    tree_walker(visit_func visit, count_func count, at_func at): visit_(visit), count_(count), at_(at) { }

    void operator()(dbgfmt::named_fmt_result node, int shift = 0) {
        visit_(node, shift);
        for (size_t i = 0; i < count_(node); ++i) {
            auto child = at_(node, i);
            (*this)(child, shift + 2);
        }
    }

private:
    visit_func visit_;
    count_func count_;
    at_func at_;
};


} // namespace cxxdbg::util::tree


namespace cxxdbg::dbg::test {


/// class watch_list_server_test
class watch_list_server_test : public watch_list_server {
public:
    /// Constructor
    explicit watch_list_server_test() {}

    /// Destructor, destroys object
    ~watch_list_server_test() override = default;

    /// Returns variables
    std::map<watch_id, dbgfmt::named_fmt_result> get_variables() override { return vars_; };

    /// Adds variable to vars map
    void add_variable(watch_id wid, const dbgfmt::named_fmt_result & info) { vars_.insert({wid, info}); }

    typedef std::map<watch_id, dbgfmt::named_fmt_result> variables_map; ///< type of variables map

    /// Sets varables
    void set_variables(variables_map && vars) { vars_ = vars; }

    /// Sets variables
    void set_variables(const variables_map & vars) { vars_ = vars; }

    /// Sets max queue size
    void set_queue_size(size_t size) { watch_queue_set_max_count(size); }

    /// Returns queue items count
    size_t queue_items_count() const { return watch_queue_items_count(); }

private:
    variables_map vars_;

    /// Invented to expand nodes, but does nothing in this test class
    void expand_node(watch_list_impl::node_id, const watch_list_impl::node_handler &) override {}

    /// Invented to collapse nodes, but does nothing in this test class
    void collapse_node(watch_list_impl::node_id, const node_handler & handl) override {}

    /// Sets format options
    void set_fmt_opts(const dbgfmt::format_options & fmt_opts, const tree_updated_handler & handl) override {}

    /// Connects to tree updated signal.
    signal_connection connect_tree_updated(const tree_updated_handler & handl) override {
        assert(false && "should not be called");
        return {};
    }
};


namespace {
class value_context_impl_stub: public dbgfmt::backend::value_context_impl, public dbgfmt::backend::type_context_impl {
public:
    /// Returns storage for target memory
    dbgfmt::data::storage & memory() override {
        assert(false && "should not be called");
        throw dbgfmt::backend::format_error{"memory() should not be called"};
    }

    /// Returns endianness of target
    dbgfmt::data::endianness endianness() const override { return dbgfmt::data::endianness::little; }

    /// Always returns nullptr
    dbgfmt::backend::type_impl * find_type(const std::string & nm) override { return nullptr; }

    /// Returns builtin type of specified kind
    dbgfmt::backend::type_impl * bt_type(dbgfmt::backend::builtin_type_impl::kind_t kind) override { return nullptr; }

    /// Makes temporary value of specified type
    std::shared_ptr<dbgfmt::backend::value_impl> make_temp_val(const dbgfmt::backend::type_impl * type) override { return {}; }

    /// Always returns static_type
    const dbgfmt::backend::type_impl * dynamic_type_at_addr(uint64_t & addr, const dbgfmt::backend::type_impl * static_type) override {
        return static_type;
    }

    /// Returns object located at specified address or nullptr if object not found
    dbgfmt::backend::debug_object_impl * obj_at_addr(uint64_t addr) override { return nullptr; }

    /// Returns symbol name located at specified address
    dbgfmt::backend::symbol_impl * sym_at_addr(uint64_t addr) override { return nullptr; }
};
}

struct watch_list_server_fixture {
private:
    value_context_impl_stub cv_impl_;
    dbgfmt::context ctx_;

public:
    watch_list_server_test server_test;

    watch_list_server_fixture() :
            ctx_(cv_impl_, cv_impl_) {
      server_test.set_queue_size(20);
    }

    fmt_result_node_list nodes;

    auto print_named_fmt_result() {
        return util::tree::tree_walker<dbgfmt::named_fmt_result>(
                [](dbgfmt::named_fmt_result node, size_t shift) {   // visit
                    std::wcout << std::wstring(shift, L' ') <<
                               L"name = " << node.name() <<
                               L" type = " << node.type() << std::endl;
                },
                [](dbgfmt::named_fmt_result & node) -> size_t { return node.childs_size(); }, // count
                [](dbgfmt::named_fmt_result & node, size_t idx) -> dbgfmt::named_fmt_result {
                    return node.child_at(idx);
                } // at
        );
    }

    // used for debug
    void print_tree_info(const watch_list_impl::tree_info & info) {
        for (auto & node : info.root_nodes()) {
            tree_info_node_printer()(node);
        }
    }

    bool compare_trees(const watch_list_impl::tree_info & info, const std::vector<test_tree_node> & nodes) {
        if (info.root_nodes().size() != nodes.size())
            return false;

        for (size_t i = 0; i < nodes.size(); ++i)
            if (!tree_node_comparator()(info.root_nodes().at(i), nodes.at(i)))
                return false;

        return true;
    }

    util::tree::tree_walker<watch_list_impl::tree_info::node> & tree_info_node_printer() {
        using node_type = watch_list_impl::tree_info::node;
        static auto printer = util::tree::tree_walker<node_type>(
                [](const node_type & node, size_t shift) {   // visit
                    std::wcout << std::wstring(shift, L' ') <<
                               L"id = " << node.id() <<
                               L" name = " << node.name() <<
                               L" type = " << node.type() << std::endl;
                },
                [](const node_type & node) -> size_t { return node.childs().size(); }, // count
                [](const node_type & node, size_t idx) -> const node_type & { return node.childs().at(idx); } // at
        );

        return printer;
    }

    using left_type = watch_list_impl::tree_info::node;
    using right_type = test_tree_node;

    util::tree::tree_comparator<left_type, right_type> & tree_node_comparator() {
        static auto comparator = util::tree::tree_comparator<left_type, right_type>(
                [](const left_type & l, const right_type & r) -> bool { // compare
                    return l.id() == r.id() && l.name() == r.name() && l.type() == r.type();
                },
                [](const left_type & l) -> size_t { return l.childs().size(); }, // left count
                [](const left_type & l, size_t i) -> const left_type & { return l.childs()[i]; }, // left get
                [](const right_type & r) -> size_t { return r.childs_size(); }, // right count
                [](const right_type & r, size_t i) -> const right_type & { return r.child_at(i); } // right get
        );

        return comparator;
    }

    auto & fmt_result_comparator() {
        static auto comparator = util::tree::tree_comparator<dbgfmt::named_fmt_result, test_tree_node>(
                [](dbgfmt::named_fmt_result l, const test_tree_node & r) -> bool {
                    return l.name() == r.name() && l.type() == r.type();
                },
                [](dbgfmt::named_fmt_result l) -> size_t { return l.childs_size(); },
                [](dbgfmt::named_fmt_result l, size_t i) -> dbgfmt::named_fmt_result { return l.child_at(i); },
                [](const right_type & r) -> size_t { return r.childs_size(); },
                [](const right_type & r, size_t i) -> const right_type & { return r.child_at(i); }
        );

        return comparator;
    }
};


BOOST_FIXTURE_TEST_SUITE(watch_list_server_test_suite, watch_list_server_fixture)

BOOST_AUTO_TEST_CASE(test_tree_comparator) {

    auto & trees_are_equal = fmt_result_comparator();
    {
        fmt_result_node r = {L"1", L"t1"};
        test_tree_node t = {1, L"1", L"t1"};
        test_tree_node t1 = {1, L"1", L"t2"};
        test_tree_node t2 = {1, L"2", L"t1"};
        test_tree_node t3 = {2, L"1", L"t1"};
        test_tree_node t4 = {1, L"1", L"t1", {{2, L"11", L"t11"}}};

        BOOST_CHECK(trees_are_equal(r, t));
        BOOST_CHECK(!trees_are_equal(r, t1));
        BOOST_CHECK(!trees_are_equal(r, t2));
        BOOST_CHECK(trees_are_equal(r, t3));
        BOOST_CHECK(!trees_are_equal(r, t4));
    }

    {
        fmt_result_node r = {L"1", L"t1", {
                {L"11", L"t11", {
                        {L"111", L"t111"}
                }},
                {L"12", L"t12"},
                {L"13", L"t13", {
                        {L"131", L"t131"},
                        {L"132", L"t132"}
                } }
        }};

        test_tree_node t =
                {1, L"1", L"t1", {
                        {2, L"11", L"t11", {
                                {3, L"111", L"t111"}
                        }},
                        {4, L"12", L"t12"},
                        {5, L"13", L"t13", {
                                {6, L"131", L"t131"},
                                {7, L"132", L"t132"}
                        } }
                }};

        test_tree_node t1 =
                {1, L"1", L"t1", {
                        {2, L"11", L"t11", {
                                {3, L"111", L"t111"}
                        }},
                        {4, L"12", L"t12"},
                        {5, L"13", L"t13", {
                                {6, L"131", L"t131"},
                                {7, L"1321", L"t132"}
                        } }
                }};

        BOOST_CHECK(trees_are_equal(r, t));
        BOOST_CHECK(!trees_are_equal(r, t1));
    }

    {
        fmt_result_node r = {L"1", L"t1", {
                {L"11", L"t11", {
                        {L"111", L"t111", {
                                {L"1111", L"t1111", {
                                        {L"11111", L"t11111"},
                                        {L"11112", L"t11112"},
                                }}
                        }}
                }},
                {L"13", L"t13", {
                        {L"131", L"t131"},
                        {L"132", L"t132"}
                } }
        }};

        test_tree_node t =
                {1, L"1", L"t1", {
                        {2, L"11", L"t11", {
                                {3, L"111", L"t111", {
                                        {4, L"1111", L"t1111", {
                                                {5, L"11111", L"t11111"},
                                                {6, L"11112", L"t11112"},
                                        }}
                                }}
                        }},
                        {7, L"13", L"t13", {
                                {8, L"131", L"t131"},
                                {9, L"132", L"t132"}
                        } }
                }};

        test_tree_node t1 =
                {1, L"1", L"t1", {
                        {2, L"11", L"t11", {
                                {3, L"111", L"t111", {
                                        {4, L"1111", L"t1111", {
                                                {5, L"11113", L"t11111"},
                                                {6, L"11112", L"t11112"},
                                        }}
                                }}
                        }},
                        {7, L"13", L"t13", {
                                {8, L"131", L"t131"},
                                {9, L"132", L"t132"}
                        } }
                }};

        BOOST_CHECK(trees_are_equal(r,t));
        BOOST_CHECK(!trees_are_equal(r, t1));
    }
}

BOOST_AUTO_TEST_CASE(test_make_values_tree_single_node) {
    auto & are_equal = tree_node_comparator();
    nodes.add(1, {L"1", L"t1"});
    server_test.set_variables(nodes.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {{1, L"1", L"t1"}}));
}

BOOST_AUTO_TEST_CASE(test_make_values_trees_two_nodes) {
    //auto & are_equal = trees_comparator();
    nodes.add(1, {L"1", L"t1"});
    nodes.add(2, {L"2", L"t2"});
    server_test.set_variables(nodes.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
        {1, L"1", L"t1"},
        {2, L"2", L"t2"}}));
}

BOOST_AUTO_TEST_CASE(test_make_values_trees_node_with_children) {
    nodes.add(1,
              {L"1", L"t1", {
                  {L"11", L"t11"},
                  {L"12", L"t12"}
    }});

    server_test.set_variables(nodes.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"},
    }));
}

BOOST_AUTO_TEST_CASE(test_make_values_trees_node_complex) {
    nodes.add(1,
              {L"1", L"t1", {
                      {L"11", L"t11", {
                              {L"111", L"t111"},
                              {L"112", L"t112"}
                      }}
              }});
    nodes.add(2,
              {L"2", L"t2", {
                      {L"21", L"t21"}
              }});
    nodes.add(3, {L"3", L"t3", {
            {L"31", L"t31", {
                    {L"311", L"t311"}
            }}
    }});
    nodes.add(4, {L"4", L"t4"});

    server_test.set_variables(nodes.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"},
            {2, L"2", L"t2"},
            {3, L"3", L"t3"},
            {4, L"4", L"t4"}
    }));
}

BOOST_AUTO_TEST_CASE(test_make_values_trees_node_expand_collapse) {
    nodes.add(1, {L"1", L"t1", {
            {L"11", L"t11", {
                    {L"111", L"t111"},
                    {L"112", L"t112"}
            }},
            {L"12", L"t12", {
                    {L"121", L"t121"}
            }},
            {L"13", L"t13"}
    }});

    // initial state
    server_test.set_variables(nodes.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"}
    }));

    // expand
    {
        watch_list_impl::tree_info::node node = server_test.expand_node_async(1);
        BOOST_CHECK(tree_node_comparator()(node, {1, L"1", L"t1", {
                {2, L"11", L"t11"},
                {3, L"12", L"t12"},
                {4, L"13", L"t13"}
        }}));

        BOOST_CHECK(compare_trees(server_test.make_values_tree(),
                                  {
                                      {1, L"1", L"t1", {
                                          {2, L"11", L"t11"}, {3, L"12", L"t12"}, {4, L"13", L"t13"}
                                      }}
                                  })
        );
    }

    // collapse
    {
        auto node = server_test.collapse_node_async(1);
        BOOST_CHECK(tree_node_comparator()(node, {1, L"1", L"t1"}));
        BOOST_CHECK(compare_trees(server_test.make_values_tree(), {{1, L"1", L"t1"}}));
    }

    // expand again
    {
        watch_list_impl::tree_info::node node = server_test.expand_node_async(1);
        BOOST_CHECK(tree_node_comparator()(node, {1, L"1", L"t1", {
                {5, L"11", L"t11"},
                {6, L"12", L"t12"},
                {7, L"13", L"t13"}
        }}));

        auto tree = server_test.make_values_tree();
        BOOST_CHECK(compare_trees(tree,
        {
        {1, L"1", L"t1", {
                    {5, L"11", L"t11"}, {6, L"12", L"t12"}, {7, L"13", L"t13"}
            }}
        }));
    }

    // expand another node
    {
        watch_list_impl::tree_info::node node = server_test.expand_node_async(5);
        BOOST_CHECK(tree_node_comparator()(node, {5, L"11", L"t11", {
                {8, L"111", L"t111"},
                {9, L"112", L"t112"},
        }}));

        auto tree = server_test.make_values_tree();
        BOOST_CHECK(compare_trees(tree, {
                {1, L"1", L"t1", {
                    {5, L"11", L"t11", {
                        {8, L"111", L"t111"}, {9, L"112", L"t112"}
                    }},

                                         {6, L"12", L"t12"},
                                         {7, L"13", L"t13"}
                }}
        }));
    }

    // expand another node
    {
        watch_list_impl::tree_info::node node = server_test.expand_node_async(6);
        BOOST_CHECK(tree_node_comparator()(node, {6, L"12", L"t12", {{10, L"121", L"t121"}}}));

        auto tree = server_test.make_values_tree();
        BOOST_CHECK(compare_trees(tree, {
                {1, L"1", L"t1", {
                    {5, L"11", L"t11", {
                        {8, L"111", L"t111"}, {9, L"112", L"t112"}
                        }}, {6, L"12", L"t12", {{10, L"121", L"t121"}}}, {7, L"13", L"t13"}
                }}
        }));
    }

    // add node
    {
        nodes.add(2, {L"2", L"t2"});
        server_test.set_variables(nodes.make_variables());
        auto tree = server_test.make_values_tree();
        BOOST_CHECK(compare_trees(tree, {
                {1, L"1", L"t1", {
                    {5, L"11", L"t11",
                            {
                                {8, L"111", L"t111"}, {9, L"112", L"t112"}
                            }}, {6, L"12", L"t12", {{10, L"121", L"t121"}}}, {7, L"13", L"t13"}
                }},
                {11, L"2", L"t2"}
        }));
    }

    // collapse first node again
    {
        watch_list_impl::tree_info::node node = server_test.collapse_node_async(1);
        BOOST_CHECK(tree_node_comparator()(node, {1, L"1", L"t1"}));

        auto tree = server_test.make_values_tree();
        BOOST_CHECK(compare_trees(tree, {
            {1, L"1", L"t1"},
            {11, L"2", L"t2"}
        }));
    }
};


BOOST_AUTO_TEST_CASE(test_watch_queue_find_by_watch_id) {
    // make a list of two var trees
    // expand one of them and then remove it - it should go to queue
    // then restore it - it should return expanded and its root id should stay the same
    nodes.add(1,
              {L"1", L"t1", {
                      {L"11", L"t11", {
                              {L"111", L"t111"},
                              {L"112", L"t112"}
                      }}
              }});
    nodes.add(2,
              {L"2", L"t2", {
                      {L"21", L"t21"}
              }});

    server_test.set_variables(nodes.make_variables());

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"},
            {2, L"2", L"t2"}}));

    server_test.expand_node_async(1);

    // compare trees
    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", {
                {3, L"11", L"t11"}
            }},
            {2, L"2", L"t2"}}));

    server_test.expand_node_async(3);

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", {
                {3, L"11", L"t11", {
                    {4, L"111", L"t111"}, {5, L"112", L"t112"}
                }}
            }},
            {2, L"2", L"t2"}}));

    // move {1} to the queue
    fmt_result_node_list nodes1;
    nodes1.add(2,
               {L"2", L"t2", {
                   {L"21", L"t21"}
               }});

    server_test.set_variables(nodes1.make_variables());

#if 0
    print_tree_info(server_test.make_values_tree());
#endif

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {2, L"2", L"t2"}
    }));

    // return back old nodes
    server_test.set_variables(nodes.make_variables());

#if 0
    print_tree_info(server_test.make_values_tree());
#endif
    // check that node 1 is fully expanded like before
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", {
                {3, L"11", L"t11", {
                    {4, L"111", L"t111"}, {5, L"112", L"t112"}
                }}
            }},
            {2, L"2", L"t2"}
    }));
}

BOOST_AUTO_TEST_CASE(test_watch_queue_find_by_name_type) {
    nodes.add(1,
              {L"1", L"t1", {
                      {L"11", L"t11", {
                              {L"111", L"t111"},
                              {L"112", L"t112"}
                      }}
              }});
    nodes.add(2,
              {L"2", L"t2", {
                      {L"21", L"t21"}
              }});

    server_test.set_variables(nodes.make_variables());

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"},
            {2, L"2", L"t2"}}));

    server_test.expand_node_async(1);

    // compare trees
    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", {{3, L"11", L"t11"}}},
            {2, L"2", L"t2"}}));

    server_test.expand_node_async(3);

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", {
                {3, L"11", L"t11", {
                    {4, L"111", L"t111"}, {5, L"112", L"t112"}
                }}
            }},
            {2, L"2", L"t2"}}));

    // move {1} to the queue
    fmt_result_node_list nodes1;
    nodes1.add(2,
               {L"2", L"t2", {
                       {L"21", L"t21"}
               }});

    server_test.set_variables(nodes1.make_variables());

#if 0
    print_tree_info(server_test.make_values_tree());
#endif

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {2, L"2", L"t2"}
    }));

    fmt_result_node_list nodes2;
    nodes2.add(2,
              {L"2", L"t2", {
                      {L"21", L"t21"}
              }});
    nodes2.add(10, // watch_id changed
               {L"1", L"t1", {
                       {L"11", L"t11", {
                               {L"111", L"t111"},
                               {L"112", L"t112"}
                       }}
               }});

    // return back old nodes
    server_test.set_variables(nodes2.make_variables());

#if 0
    print_tree_info(server_test.make_values_tree());
#endif

    // check that node 1 is fully expanded like before
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {2, L"2", L"t2"},
            {1 /* node id is the same = 1*/, L"1", L"t1", {
                {3, L"11", L"t11", {
                    {4, L"111", L"t111"}, {5, L"112", L"t112"}
                    }}
            }}
    }));
}

BOOST_AUTO_TEST_CASE(test_watch_queue_max_size) {
    nodes.add(1, {L"1", L"t1", { {L"11", L"t11"} }});
    nodes.add(2, {L"2", L"t2", { {L"21", L"t21"} }});
    nodes.add(3, {L"3", L"t3", { {L"31", L"t31"} }});
    nodes.add(4, {L"4", L"t4", { {L"41", L"t41"} }});

    server_test.set_variables(nodes.make_variables());

    BOOST_REQUIRE(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1"}, {2, L"2", L"t2"}, {3, L"3", L"t3"}, {4, L"4", L"t4"}
    }));

    server_test.expand_node_async(1);
    server_test.expand_node_async(2);
    server_test.expand_node_async(3);
    server_test.expand_node_async(4);

//    print_tree_info(server_test.make_values_tree());

    server_test.set_queue_size(2);

    fmt_result_node_list nodes1;
    nodes1.add(1, {L"1", L"t1", { {L"11", L"t11"} }});
    nodes1.add(2, {L"2", L"t2", { {L"21", L"t21"} }});

    server_test.set_variables(nodes1.make_variables());
    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", { {5, L"11", L"t11"} }}, {2, L"2", L"t2", { {6, L"21", L"t21"} }}
    }));

    BOOST_CHECK(server_test.queue_items_count() == 2);

    fmt_result_node_list nodes2;
    nodes2.add(1, {L"1", L"t1", { {L"11", L"t11"} }});

    server_test.set_variables(nodes2.make_variables());

    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", { {5, L"11", L"t11"} }}
    }));

    BOOST_CHECK(server_test.queue_items_count() == 2);

    server_test.set_variables(nodes.make_variables()); // return back old var list

#if 0
    print_tree_info(server_test.make_values_tree());
#endif

    BOOST_CHECK(compare_trees(server_test.make_values_tree(), {
            {1, L"1", L"t1", { {5, L"11", L"t11"} }},
            {2, L"2", L"t2", { {6, L"21", L"t21"} }},
            {9, L"3", L"t3"}, // 3-rd node is not expanded because it was removed from queue and then created anew
            {4, L"4", L"t4", { {8, L"41", L"t41"} }}
    }));
    // node id 9 is in place of 3 because watch id stayed the same = 3

    BOOST_CHECK(server_test.queue_items_count() == 0);
}


BOOST_AUTO_TEST_SUITE_END()


} // namespace cxxdbg::dbg::test
