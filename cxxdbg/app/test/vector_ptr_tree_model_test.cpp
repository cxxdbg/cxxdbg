// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file vector_ptr_tree_model_test.cpp
/// Contains unit tests for the vector_ptr_tree_model class.

#include "../vector_ptr_model.hpp"
#include "../vector_ptr_tree_model.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::test {


namespace {

class my_item_base {
public:
    virtual ~my_item_base() = default;
};

class my_item;

class my_item_child: public my_item_base {
public:
    my_item_child(my_item * p): parent_{p} {}
    ~my_item_child() override = default;
    const my_item * parent() const { return parent_; }

private:
    my_item * parent_;
};

class my_item: public my_item_base {
public:
    ~my_item() override = default;

    void add_child() {
        childs_.push_back(my_item_child{this});
    }

    auto & childs() const { return childs_; }

    void clear_childs() { childs_.clear(); }

private:
    std::vector<my_item_child> childs_;
};

struct my_converter {
    size_t childs_size(const my_item_base * item) const {
        auto ritem = dynamic_cast<const my_item*>(item);
        if (ritem == nullptr) {
            // child item
            return 0;
        }

        return ritem->childs().size();
    }

    const my_item_base * child(const my_item_base * item, size_t idx) const {
        assert(idx < childs_size(item) && "invalid child index");
        auto ritem = dynamic_cast<const my_item*>(item);
        assert(ritem != nullptr && "can't get child for child item");
        return &ritem->childs().at(idx);
    }

    /// Always return nullptr (all items are in root)
    const my_item_base * parent(const my_item_base * item) const {
        auto chld = dynamic_cast<const my_item_child*>(item);
        if (chld == nullptr) {
            // root node
            return nullptr;
        }

        return chld->parent();
    }

    /// Returns index of item in parent. Should never be called.
    size_t index(const my_item_base * item) const {
        auto chld = dynamic_cast<const my_item_child*>(item);
        assert(chld && "can't get index for root item");
        auto p = chld->parent();
        auto it = std::find_if(p->childs().begin(), p->childs().end(), [item](const auto & val) {
            return &val == item;
        });
        assert(it != p->childs().end() && "can't find child item");
        auto idx = std::distance(p->childs().begin(), it);
        assert(idx > 0 && "invalid value returned by distance");
        return static_cast<size_t>(idx);
    }
};

}


struct vector_ptr_tree_model_test_fixture {
    using my_vector_model = vector_ptr_model<my_item>;
    my_vector_model vmodel;
    vector_ptr_tree_model<my_vector_model, my_item_base, my_converter> tmodel{vmodel};
};


BOOST_FIXTURE_TEST_SUITE(vector_ptr_tree_model_test, vector_ptr_tree_model_test_fixture)


/// Tests adding new item
BOOST_AUTO_TEST_CASE(add_item) {

    {
        auto first = std::make_unique<my_item>();
        first->add_child();
        first->add_child();
        vmodel.push_back(std::move(first));
    }

    {
        auto second = std::make_unique<my_item>();
        second->add_child();
        vmodel.push_back(std::move(second));
    }

    BOOST_CHECK_EQUAL(tmodel.childs_size(nullptr), 2);

    int n_before_added_called = 0;
    tmodel.before_added.connect(
    [&n_before_added_called, this](const my_item_base * parent, size_t first, size_t last) {
        ++n_before_added_called;

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 2);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 1);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
    });

    int n_after_added_called = 0;
    tmodel.after_added.connect(
    [&n_after_added_called, this](const my_item_base * parent, size_t first, size_t last) {
        ++n_after_added_called;

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 3);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 3);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
        BOOST_CHECK(tmodel.child(root2, 1) == &vmodel[1]->childs()[1]);
        BOOST_CHECK(tmodel.child(root2, 2) == &vmodel[1]->childs()[2]);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
    });

    tmodel.before_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    tmodel.after_removed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    tmodel.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    tmodel.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    {
        auto new_second = std::make_unique<my_item>();
        new_second->add_child();
        new_second->add_child();
        new_second->add_child();
        vmodel.insert(1, std::move(new_second));
    }

    BOOST_CHECK_EQUAL(n_before_added_called, 1);
    BOOST_CHECK_EQUAL(n_after_added_called, 1);

    BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 3);

    auto root1 = tmodel.child(nullptr, 0);
    BOOST_CHECK(root1 == vmodel[0]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
    BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
    BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

    auto root2 = tmodel.child(nullptr, 1);
    BOOST_CHECK(root2 == vmodel[1]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 3);
    BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
    BOOST_CHECK(tmodel.child(root2, 1) == &vmodel[1]->childs()[1]);
    BOOST_CHECK(tmodel.child(root2, 2) == &vmodel[1]->childs()[2]);

    auto root3 = tmodel.child(nullptr, 2);
    BOOST_CHECK(root3 == vmodel[2]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
    BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
}


/// Tests removing item
BOOST_AUTO_TEST_CASE(remove_item) {

    {
        auto first = std::make_unique<my_item>();
        first->add_child();
        first->add_child();
        vmodel.push_back(std::move(first));
    }

    {
        auto old_second = std::make_unique<my_item>();
        old_second->add_child();
        old_second->add_child();
        old_second->add_child();
        vmodel.insert(1, std::move(old_second));
    }

    {
        auto second = std::make_unique<my_item>();
        second->add_child();
        vmodel.push_back(std::move(second));
    }

    BOOST_CHECK_EQUAL(tmodel.childs_size(nullptr), 3);

    tmodel.before_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    tmodel.after_added.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    int n_before_removed_called = 0;
    tmodel.before_removed.connect(
    [this, &n_before_removed_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_before_removed_called;

        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 1);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 3);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 3);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
        BOOST_CHECK(tmodel.child(root2, 1) == &vmodel[1]->childs()[1]);
        BOOST_CHECK(tmodel.child(root2, 2) == &vmodel[1]->childs()[2]);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
    });

    int n_after_removed_called = 0;
    tmodel.after_removed.connect(
    [this, &n_after_removed_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_after_removed_called;

        BOOST_CHECK(parent == nullptr);
        BOOST_CHECK_EQUAL(first, 1);
        BOOST_CHECK_EQUAL(last, 1);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 2);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 1);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
    });

    tmodel.before_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });

    tmodel.after_changed.connect([](auto && ...) {
        BOOST_CHECK(false);
    });


    vmodel.erase(1);


    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);

    BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 2);

    auto root1 = tmodel.child(nullptr, 0);
    BOOST_CHECK(root1 == vmodel[0]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
    BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
    BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

    auto root2 = tmodel.child(nullptr, 1);
    BOOST_CHECK(root2 == vmodel[1]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 1);
    BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
}


/// Tests changing item
BOOST_AUTO_TEST_CASE(change_item) {

    {
        auto first = std::make_unique<my_item>();
        first->add_child();
        first->add_child();
        vmodel.push_back(std::move(first));
    }

    {
        auto second = std::make_unique<my_item>();
        second->add_child();
        second->add_child();
        second->add_child();
        vmodel.insert(1, std::move(second));
    }

    {
        auto third = std::make_unique<my_item>();
        third->add_child();
        vmodel.push_back(std::move(third));
    }

    BOOST_CHECK_EQUAL(tmodel.childs_size(nullptr), 3);

    int n_before_added_called = 0;
    tmodel.before_added.connect(
    [this, &n_before_added_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_before_added_called;

        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(parent == vmodel[1]);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(parent), 0);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 0);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);

        BOOST_CHECK_EQUAL(vmodel[1]->childs().size(), 1);
    });

    int n_after_added_called = 0;
    tmodel.after_added.connect(
    [this, &n_after_added_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_after_added_called;

        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(parent == vmodel[1]);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 0);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(parent), 1);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 1);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);

        BOOST_CHECK_EQUAL(vmodel[1]->childs().size(), 1);
    });

    int n_before_removed_called = 0;
    tmodel.before_removed.connect(
    [this, &n_before_removed_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_before_removed_called;

        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(parent == vmodel[1]);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(parent), 3);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 3);
        BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);
        BOOST_CHECK(tmodel.child(root2, 1) == &vmodel[1]->childs()[1]);
        BOOST_CHECK(tmodel.child(root2, 2) == &vmodel[1]->childs()[2]);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
    });

    int n_after_removed_called = 0;
    tmodel.after_removed.connect(
    [this, &n_after_removed_called](const my_item_base * parent, size_t first, size_t last) {
        ++n_after_removed_called;

        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(parent == vmodel[1]);
        BOOST_CHECK_EQUAL(first, 0);
        BOOST_CHECK_EQUAL(last, 2);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(parent), 0);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 0);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
    });

    int n_before_changed_called = 0;
    tmodel.before_changed.connect(
    [this, &n_before_changed_called](const my_item_base * node) {
        ++n_before_changed_called;

        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(node == vmodel[1]);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(node), 0);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 0);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);

        BOOST_CHECK_EQUAL(vmodel[1]->childs().size(), 3);
    });

    int n_after_changed_called = 0;
    tmodel.after_changed.connect(
    [this, &n_after_changed_called](const my_item_base * node) {
        ++n_after_changed_called;


        BOOST_REQUIRE_EQUAL(vmodel.size(), 3);

        BOOST_CHECK(node == vmodel[1]);

        BOOST_REQUIRE_EQUAL(tmodel.childs_size(node), 0);

        auto root1 = tmodel.child(nullptr, 0);
        BOOST_CHECK(root1 == vmodel[0]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
        BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
        BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

        auto root2 = tmodel.child(nullptr, 1);
        BOOST_CHECK(root2 == vmodel[1]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 0);

        auto root3 = tmodel.child(nullptr, 2);
        BOOST_CHECK(root3 == vmodel[2]);
        BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
        BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);

        BOOST_CHECK_EQUAL(vmodel[1]->childs().size(), 1);
    });


    {
        auto changer = vmodel.begin_change(1);
        changer->clear_childs();
        changer->add_child();
    }

    BOOST_CHECK_EQUAL(n_before_removed_called, 1);
    BOOST_CHECK_EQUAL(n_after_removed_called, 1);

    BOOST_REQUIRE_EQUAL(tmodel.childs_size(nullptr), 3);

    auto root1 = tmodel.child(nullptr, 0);
    BOOST_CHECK(root1 == vmodel[0]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root1), 2);
    BOOST_CHECK(tmodel.child(root1, 0) == &vmodel[0]->childs()[0]);
    BOOST_CHECK(tmodel.child(root1, 1) == &vmodel[0]->childs()[1]);

    auto root2 = tmodel.child(nullptr, 1);
    BOOST_CHECK(root2 == vmodel[1]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root2), 1);
    BOOST_CHECK(tmodel.child(root2, 0) == &vmodel[1]->childs()[0]);

    auto root3 = tmodel.child(nullptr, 2);
    BOOST_CHECK(root3 == vmodel[2]);
    BOOST_REQUIRE_EQUAL(tmodel.childs_size(root3), 1);
    BOOST_CHECK(tmodel.child(root3, 0) == &vmodel[2]->childs()[0]);
}


BOOST_AUTO_TEST_SUITE_END()


}
