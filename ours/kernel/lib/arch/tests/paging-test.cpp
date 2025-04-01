#include <arch/paging.hpp>
#include <arch/paging/x86_pagings.hpp>

#include <random>
#include <memory>
#include <map>

#include <gtest/gtest.h>
#include <ustl/array.hpp>
#include <ustl/util/enum_sequence.hpp>

using namespace arch;
using namespace paging;
using ArchPaging = Paging<PagingDispatcher<X86PagingLevel::PageMapLevel4>::Type>;

using TestingTypes = ::testing::Types<ArchPaging>;

struct PagingTestFixture
    : public testing::Test {

    static inline auto p2v = [] (auto x) { return x; };

    inline auto alloc_page_table(usize size, usize alignment) -> PhysAddr {
        // Should never be called.
        if (level >= usize(ArchPaging::kPagingLevel)) {
            std::terminate();
        }

        if (level) {
            return PhysAddr(pt_storage[level--]);
        }
        level = usize(ArchPaging::kPagingLevel) - 1;
        return PhysAddr(pt_storage[0]);
    }

    template <ArchPaging::LevelType TopLevel>
    struct GetPageTableStorage {
        template <typename>
        struct GetPageTableStorageImpl;

        template <ArchPaging::LevelType... Levels>
        struct GetPageTableStorageImpl<ustl::EnumSequence<ArchPaging::LevelType, Levels...>> {
            // Levels must be a increasing sequence
            using Type = std::tuple<ArchPaging::Table<Levels> *...>;
        };

        using Type = typename GetPageTableStorageImpl<
            ustl::MakeEnumSequenceT<
                ArchPaging::LevelType, 
                usize(ArchPaging::kFinalLevel), 
                usize(TopLevel) + 1
            >
        >::Type;
    };

    template <ArchPaging::LevelType Level>
    auto build_page_table() {
        auto table = new (std::align_val_t(ArchPaging::kTableAlignment)) ArchPaging::Table<Level>;
        pt_storage[usize(Level)] = table;
        // TODO(SmallHuaZi): Change MMU flags to use ArchMmuFlags.
        if constexpr (Level == ArchPaging::kFinalLevel) {
            for (auto i = 0; i < table->size(); ++i) {
                (*table)[i] = ArchPaging::Pte<Level>((i * PAGE_SIZE) | X86_KERNEL_PAGE_FLAGS);
            }
        } else {
            auto next_level_table = build_page_table<ArchPaging::next_level(Level)>();
            for (auto i = 0; i < table->size(); ++i) {
                (*table)[i] = ArchPaging::Pte<Level>(PhysAddr(next_level_table) | X86_KERNEL_PD_FLAGS);
            }
        }

        return PhysAddr(table);
    }

    auto SetUp() -> void final {
        pgd = build_page_table<ArchPaging::kPagingLevel>();
    }

    auto TearDown() -> void final
    {}

    auto unmap_all_terminal() -> void {
        memset(pt_storage[usize(ArchPaging::kFinalLevel)], 0,
              ArchPaging::kSizeOfTable<ArchPaging::kFinalLevel>);
    }

    auto map(VirtAddr va, usize n, PhysAddr pa, MmuFlags flags) {
        auto allocator = std::bind_front(&PagingTestFixture::alloc_page_table, this);
        return ArchPaging::map(pgd, p2v, allocator, va, pa, n, flags);
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

    usize level = usize(ArchPaging::kPagingLevel) - 1;
    PhysAddr pgd;
    // Holding the page table item each level.
    // The first element is the page table of the lowest level.
    void *pt_storage[usize(ArchPaging::kPagingLevel) + 1]{};
};

static auto to_string(MmuFlags mmuflags) -> std::string {
    std::string out = "r|";
    if (bool(mmuflags & MmuFlags::Executable)) {
        out += "x|";
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

TEST_F(PagingTestFixture, Query) {
    for (auto i = 0; i < 512; ++i) {
        auto const result = query(i * PAGE_SIZE);
        ASSERT_EQ(result->phys, i * PAGE_SIZE)
            << "p: " << result->phys;

        ASSERT_EQ(result->flags, MmuFlags::PermMask | MmuFlags::Present)
            << "f: " << to_string(result->flags);
    }

    VirtAddr const upper_va = -MB(2);
    for (auto i = 0; i < 512; ++i) {
        auto const result = query(upper_va  + i * PAGE_SIZE);
        ASSERT_EQ(result->phys, i * PAGE_SIZE)
            << "p: " << result->phys;

        ASSERT_EQ(result->flags, MmuFlags::PermMask | MmuFlags::Present)
            << "f: " << to_string(result->flags);
    }
}

TEST_F(PagingTestFixture, RandomizedMapAndQuery) {
    auto nr_mappings = 20;
    while (nr_mappings--) {
        unmap_all_terminal();

        auto [virt, phys] = random_mapping(0, GB(16), 0, GB(1));
        auto option = map(virt, 1, phys, MmuFlags::PermMask | MmuFlags::Present);
        if (!(option && option.value() == MapError::None)) {
            map(virt, 1, phys, MmuFlags::PermMask | MmuFlags::Present);
            ASSERT_TRUE(option && option.value() == MapError::None);
        }
        auto const result = query(virt);
        ASSERT_TRUE(result) << "Map";
        ASSERT_TRUE(result->flags == (MmuFlags::PermMask | MmuFlags::Present)) 
            << "f: " << to_string(result->flags) << ", nr_mappings: " << nr_mappings;
        ASSERT_EQ(result->phys, phys) 
            << "p: " << result->phys 
            << ", v: " << virt
            << ", nr_mappings: " << nr_mappings;
    }
}