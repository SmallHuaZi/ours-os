#include <arch/paging.hpp>
#include <arch/paging/x86_pagings.hpp>

#include <random>
#include <memory>
#include <map>

#include <gtest/gtest.h>

using namespace arch;
using namespace paging;
using ArchPaging = Paging<PagingDispatcher<X86PagingLevel::PageMapLevel4>::Type>;

using TestingTypes = ::testing::Types<ArchPaging>;

struct PagingTestFixture
    : public testing::Test {

    static inline std::vector<PhysAddr> free_pts{};
    static inline std::vector<PhysAddr> used_pts{};

    static inline auto p2v = [] (auto x) { return x; };
    static inline auto alloc_page_table = [] (std::size_t size, std::size_t alignment) -> PhysAddr {
        if (free_pts.empty()) {
            used_pts.push_back(reinterpret_cast<PhysAddr>(new (std::align_val_t(alignment)) char[size]));
        } else {
            used_pts.push_back(free_pts.back());
            free_pts.pop_back();
        }
        return used_pts.back();
    };
    
    auto SetUp() -> void final {
        pgd = alloc_page_table(PAGE_SIZE, PAGE_SIZE);
    }

    auto TearDown() -> void final {
        std::swap(used_pts, free_pts);
    }

    auto map(VirtAddr va, usize n, PhysAddr pa, MmuFlags flags) -> void {
        ArchPaging::map(pgd, p2v, alloc_page_table, va, pa, n, flags);
    }

    auto query(VirtAddr va) -> ustl::Option<QueryResult> {
        return ArchPaging::query(pgd, p2v, va);
    }

    auto random_mapping(VirtAddr vstart, VirtAddr vend, PhysAddr pstart, PhysAddr pend, usize page_size = PAGE_SIZE) 
        -> std::pair<VirtAddr, PhysAddr> 
    {
        std::random_device device;
        std::uniform_int_distribution<VirtAddr> uid(vstart, vend - 1);
        VirtAddr virt = ((uid(device) + page_size - 1) & ~(page_size - 1));

        uid.param(decltype(uid)::param_type{pstart, pend - 1});
        PhysAddr phys = ((uid(device) + page_size - 1) & ~(page_size - 1));
        return std::make_pair(virt, phys);
    }

    PhysAddr pgd;
};

static auto to_string(MmuFlags mmuflags) -> std::string {
    std::string out;
    if (bool(mmuflags & MmuFlags::Executable)) {
        out += "x|";
    }
    if (bool(mmuflags & MmuFlags::Readable)) {
        out += "r|";
    }
    if (bool(mmuflags & MmuFlags::Writable)) {
        out += "w|";
    }
    if (!bool(mmuflags & MmuFlags::Discache)) {
        out += "c|";
    }
    if (!out.empty()) {
        out.pop_back();
    }

    return std::move(out);
}

namespace ours {
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void {
        while (1);
    }
}

TEST_F(PagingTestFixture, RandomizedMapAndQuery) {
    auto nr_mappings = 20;
    while (nr_mappings--) {
        auto [virt, phys] = random_mapping(0, GB(16), 0, GB(1));
        map(virt, 1, phys, MmuFlags::PermMask | MmuFlags::Present);
        auto const result = query(virt);
        ASSERT_TRUE(result) << "Map";
        ASSERT_TRUE(result->flags == (MmuFlags::PermMask | MmuFlags::Present)) 
            << "f: " << to_string(result->flags);
        ASSERT_EQ(result->phys, phys) 
            << "p: " << result->phys;
    }
}