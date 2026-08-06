// Copyright (c) 2026, Alexandr Esilevich
//
// Licensed under the Apache License, Version 2.0.
// See accompanying file LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0
//

/// \file disassembler_test.cpp
/// Contains unit tests for disassembler.

#include "../memory_disassembler.hpp"
#include <boost/test/unit_test.hpp>


namespace cxxdbg::dbg::core {


class mock_disassembler: public disassembler {
public:
    /// Constructs mock disassembler with specified max instruction byte size,
    /// mock instruction byte size and address of start of memory block
    mock_disassembler(size_t max_inst_s, size_t mock_inst_s, uint64_t s_addr):
        max_inst_size_{max_inst_s}, mock_inst_size_{mock_inst_s}, start_addr_{s_addr} {}

    /// Default virtual destructor
    ~mock_disassembler() override = default;

    /// Returns maximum size of instruction in bytes
    size_t max_inst_byte_size() const override {
        return max_inst_size_;
    }

    /// Disassembles memory into list of instructions. Splits data into chunks of
    /// size equal to mock instruction size.
    std::vector<instruction> disassemble(uint64_t start_addr, uint64_t size) const override {
        // reading memory
        std::vector<char> data = read_memory(start_addr, size);
        assert(data.size() == size && "invalid data size");

        // disassembling memory
        return disassemble_data(start_addr, data.data(), size);
    }

    /// Sets memory data
    void set_data(const std::vector<char> & d) {
        data_ = d;
    }

    /// Sets memory data
    void set_data(const std::string & d) {
        set_data(std::vector<char>{d.begin(), d.end()});
    }

    /// Adds object into memory
    void add_object(const disassembler_object & obj) {
        objects_.emplace(obj.addr(), obj);
    }

    /// Returns pointer (word) size for target architecture
    size_t ptr_size() const override {
        return 4;
    }

    /// Returns object located at specified address or invalid object
    disassembler_object obj_at_addr(uint64_t addr) const override {
        if (objects_.empty()) {
            return {};
        }

        auto it = objects_.upper_bound(addr);
        if (it == objects_.begin()) {
            // the first object is located after specified addres
            return {};
        }

        --it;

        // checking that address is located inside object
        assert(it->first == it->second.addr() && "object address inconsistency");
        assert(it->first <= addr && "invalid iterator returned from upper_bound");
        if (it->second.addr() + it->second.size() <= addr) {
            return {};
        }

        return it->second;
    }

    /// Sets memory start address
    void set_start_addr(size_t a) {
        start_addr_ = a;
    }

    /// Returns memory start address
    uint64_t start_addr() const {
        return start_addr_;
    }

private:
    /// Disassembles data into list of instructions. Splits data into chunks of
    /// size equal to mock instruction size.
    std::vector<instruction> disassemble_data(uint64_t start_addr, const char * data, size_t size) const {
        std::vector<instruction> insts;

        // building list of full-size instructions
        auto full_inst_size = size / mock_inst_size_;
        for (size_t inst_i = 0; inst_i < full_inst_size; ++inst_i) {
            std::vector<char> inst_data{data + inst_i * mock_inst_size_, data + (inst_i + 1) * mock_inst_size_};
            std::string inst_str{inst_data.begin(), inst_data.end()};
            uint64_t inst_addr = start_addr + inst_i * mock_inst_size_;
            insts.push_back(instruction{inst_addr, inst_data, inst_str});
        }

        // adding small instruction with rest of data
        auto small_inst_size = size % mock_inst_size_;
        if (small_inst_size != 0) {
            std::vector<char> inst_data{data + full_inst_size * mock_inst_size_,
                                        data + full_inst_size * mock_inst_size_ + small_inst_size};
            std::string inst_str{inst_data.begin(), inst_data.end()};
            uint64_t inst_addr = start_addr + full_inst_size * mock_inst_size_;
            insts.push_back(instruction{inst_addr, inst_data, inst_str});
        }

        return insts;
    }

    /// Reads memory. The size of returned memory data is always equal to the size
    /// parameter. If memory can't be read then it's replaced with zeroes.
    std::vector<char> read_memory(uint64_t addr, uint64_t size) const {
        std::vector<char> res;

        // checking for the end of memory
        if (UINT64_MAX - addr < size) {
            size = UINT64_MAX - addr;
        }


        // filling beginning of the result with zeroes

        if (addr < start_addr_) {
            auto begin_sz = std::min(start_addr_ - addr, size);
            for (size_t i = 0; i < begin_sz; ++i) {
                res.push_back(0);
            }
        }

        assert(res.size() <= size && "invalid result size");
        if (size == res.size()) {
            return res;
        }


        // filling middle of the result with memory data

        uint64_t data_offset = 0;
        if (addr > start_addr_) {
            data_offset = addr - start_addr_;
        }

        if (data_offset < data_.size()) {
            auto data_sz = std::min(size - res.size(), data_.size() - data_offset);
            std::copy(data_.begin() + data_offset, data_.begin() + data_offset + data_sz, std::back_inserter(res));
        }

        assert(res.size() <= size && "invalid result size");
        if (size == res.size()) {
            return res;
        }


        // filling end of the result with zeroes

        auto end_sz = size - res.size();
        for (size_t i = 0; i < end_sz; ++i) {
            res.push_back(0);
        }

        return res;
    }


    size_t max_inst_size_;                              ///< Max instructions size
    size_t mock_inst_size_;                             ///< Mock instruction size
    uint64_t start_addr_;                               ///< Start address
    std::map<uint64_t, disassembler_object> objects_;   ///< Memory objects
    std::vector<char> data_;                            ///< Memory data
};


struct disassembler_test_fixture {
    mock_disassembler dis{3, 3, 10};
    memory_disassembler mem_dis{dis};
};


BOOST_FIXTURE_TEST_SUITE(disassembler_test, disassembler_test_fixture)


/// Tests disassembling region without symbols
BOOST_AUTO_TEST_CASE(no_symbols) {
    dis.set_data("this is simple memory region");

    auto res = mem_dis.disassemble(20, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{12,  {'i', 's', ' '},    "is "}));
    BOOST_CHECK((res[1] == instruction{15,  {'i', 's', ' '},    "is "}));
    BOOST_CHECK((res[2] == instruction{18,  {'s', 'i', 'm'},    "sim"}));
    BOOST_CHECK((res[3] == instruction{21,  {'p', 'l', 'e'},    "ple"}));
}


/// Tests disassembling beginning of memory without symbols
BOOST_AUTO_TEST_CASE(no_symbols_begin) {
    dis.set_data("this is simple memory region");
    dis.set_start_addr(0);

    auto res = mem_dis.disassemble(2, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{0,   {'t', 'h', 'i'},    "thi"}));
    BOOST_CHECK((res[1] == instruction{3,   {'s', ' ', 'i'},    "s i"}));
    BOOST_CHECK((res[2] == instruction{6,   {'s', ' ', 's'},    "s s"}));
    BOOST_CHECK((res[3] == instruction{9,   {'i', 'm', 'p'},    "imp"}));
}


/// Tests disassembling end of memory without symbols
BOOST_AUTO_TEST_CASE(no_symbols_end) {
    std::string str = "this is simple memory region";
    dis.set_data(str);
    dis.set_start_addr(UINT64_MAX - str.size());

    auto res = mem_dis.disassemble(UINT64_MAX - 6, 0, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 3);

    BOOST_CHECK((res[0] == instruction{UINT64_MAX - 8,   {'y', ' ', 'r'},    "y r"}));
    BOOST_CHECK((res[1] == instruction{UINT64_MAX - 5,   {'e', 'g', 'i'},    "egi"}));
    BOOST_CHECK((res[2] == instruction{UINT64_MAX - 2,   {'o', 'n'},         "on"}));
}


/// Tests disassembling memory between two objects
BOOST_AUTO_TEST_CASE(between_symbols) {
    std::string str = "this is simple memory region";
    //                 |-|       ^   |-----|
    dis.set_data(str);
    dis.add_object({dis.start_addr() +  0, 3, "first object"});
    dis.add_object({dis.start_addr() + 14, 5, "second object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 10, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() +  3,   {'s', ' ', 'i'},    "s i"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() +  6,   {'s', ' ', 's'},    "s s"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() +  9,   {'i', 'm', 'p'},    "imp"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 12,   {'l', 'e'},         "le"}));
}


/// Tests disassembling memory between two objects with object at the end of empty space
BOOST_AUTO_TEST_CASE(between_symbols_object_at_end) {
    std::string str = "this is simple memory region";
    //                 |-|       ^   |-----|
    dis.set_data(str);
    dis.add_object({dis.start_addr() +  0, 3, "first object"});
    dis.add_object({dis.start_addr() + 14, 5, "second object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 10, -2, 5);

    BOOST_REQUIRE_EQUAL(res.size(), 5);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() +  3,   {'s', ' ', 'i'},    "s i"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() +  6,   {'s', ' ', 's'},    "s s"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() +  9,   {'i', 'm', 'p'},    "imp"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 12,   {'l', 'e'},         "le"}));
    BOOST_CHECK((res[4] == instruction{dis.start_addr() + 14,   {' ', 'm', 'e'},    " me",      "second object"}));
}


/// Tests disassembling memory between two objects with object at the begining
BOOST_AUTO_TEST_CASE(between_symbols_object_at_begin) {
    std::string str = "this is simple memory region";
    //                 |--|   ^      |-----|
    dis.set_data(str);
    dis.add_object({dis.start_addr() +  0, 4, "first object"});
    dis.add_object({dis.start_addr() + 14, 5, "second object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 7, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() + 3,    {'s'},              "s"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() + 4,    {' ', 'i', 's'},    " is"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() + 7,    {' ', 's', 'i'},    " si"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 10,   {'m', 'p', 'l'},    "mpl"}));
}


/// Tests disassembling memory between two objects with object at the begining and at the end
BOOST_AUTO_TEST_CASE(between_symbols_object_at_begin_end) {
    std::string str = "this is simple memory region";
    //                 |--|   ^    |-----|
    dis.set_data(str);
    dis.add_object({dis.start_addr() +  0, 4, "first object"});
    dis.add_object({dis.start_addr() + 12, 5, "second object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 7, -2, 5);

    BOOST_REQUIRE_EQUAL(res.size(), 5);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() + 3,    {'s'},              "s"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() + 4,    {' ', 'i', 's'},    " is"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() + 7,    {' ', 's', 'i'},    " si"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 10,   {'m', 'p'},         "mp"}));
    BOOST_CHECK((res[4] == instruction{dis.start_addr() + 12,   {'l', 'e', ' '},    "le ",          "second object"}));
}


/// Tests disassembling memory inside object
BOOST_AUTO_TEST_CASE(inside_object) {
    std::string str = "this is simple memory region";
    //                  |------^------------|
    dis.set_data(str);
    dis.add_object({dis.start_addr() + 1, 21, "big object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 8, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() + 1,    {'h', 'i', 's'},    "his",      "big object"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() + 4,    {' ', 'i', 's'},    " is"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() + 7,    {' ', 's', 'i'},    " si"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 10,   {'m', 'p', 'l'},    "mpl"}));
}


/// Tests disassembling memory inside object with another object at begin
BOOST_AUTO_TEST_CASE(inside_object_obj_begin) {
    std::string str = "this is simple memory region";
    //                 ||------^------------|
    dis.set_data(str);
    dis.add_object({dis.start_addr() + 0, 1, "small object"});
    dis.add_object({dis.start_addr() + 1, 21, "big object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 8, -3, 5);

    BOOST_REQUIRE_EQUAL(res.size(), 5);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() + 0,    {'t'},              "t",        "small object"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() + 1,    {'h', 'i', 's'},    "his",      "big object"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() + 4,    {' ', 'i', 's'},    " is"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 7,    {' ', 's', 'i'},    " si"}));
    BOOST_CHECK((res[4] == instruction{dis.start_addr() + 10,   {'m', 'p', 'l'},    "mpl"}));
}


/// Tests disassembling memory inside object wtih another object at end
BOOST_AUTO_TEST_CASE(inside_object_obj_at_end) {
    std::string str = "this is simple memory region";
    //                  |------||-----------|
    //                         ^
    dis.set_data(str);
    dis.add_object({dis.start_addr() + 1, 8, "first object"});
    dis.add_object({dis.start_addr() + 9, 13, "second object"});

    auto res = mem_dis.disassemble(dis.start_addr() + 8, -2, 4);

    BOOST_REQUIRE_EQUAL(res.size(), 4);

    BOOST_CHECK((res[0] == instruction{dis.start_addr() + 1,    {'h', 'i', 's'},    "his",      "first object"}));
    BOOST_CHECK((res[1] == instruction{dis.start_addr() + 4,    {' ', 'i', 's'},    " is"}));
    BOOST_CHECK((res[2] == instruction{dis.start_addr() + 7,    {' ', 's'},         " s"}));
    BOOST_CHECK((res[3] == instruction{dis.start_addr() + 9,    {'i', 'm', 'p'},    "imp",      "second object"}));
}


BOOST_AUTO_TEST_SUITE_END()


}
