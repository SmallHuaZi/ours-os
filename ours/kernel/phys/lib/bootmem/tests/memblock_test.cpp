#include <bootmem/memblock.hpp>
#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <format>

using bootmem::PhysAddr;
using bootmem::Region;
using bootmem::RegionType;
using bootmem::RegionVector;

class MemBlockTestFixture
    : public ::testing::Test 
{
protected:
    auto SetUp() -> void override;

    auto TearDown() -> void override
    {}

    auto validate_deallocation(PhysAddr start, usize size, PhysAddr const final_allocated) -> bool;

    auto random_integer(int start, int end) -> int
    {
        std::random_device device;
        std::uniform_int_distribution<> uid(start, end);
        return uid(device);
    }

    // This union is a convenient way for debug 
    union {
        struct {
            Region memories[100];
            Region reserved[100];
        };
        Region total[200];

    } bootstrap_regions;

    using MemBlock = bootmem::MemBlock<>;
    MemBlock memblock;

    struct Action {
        PhysAddr base;
        PhysAddr size;
    };
    std::vector<Action> action;
    std::vector<Action> freepath;

    MemBlock::RegionList *reserved_list;
    MemBlock::RegionList *memories_list;

    static Region PRESET_REGIONS[];
};

Region MemBlockTestFixture::PRESET_REGIONS[] = {
    { 0x1000, 0xFFFF, RegionType::Normal, 0 },
    { 0xF'FFFF, 0x10'0000, RegionType::Normal, 0 },
    { 0xFFF'FFFF, 0x100'0000, RegionType::Normal, 0 },
    { 0x1200'0000, 0xFF'FFFF, RegionType::Normal, 0 },
};

auto MemBlockTestFixture::SetUp() -> void
{   
    for (auto &region : bootstrap_regions.reserved) {
        memset(&region, 0, sizeof(region));
    }
    RegionVector memories(&memblock), reserved(&memblock);
    memories.reset(bootstrap_regions.memories, std::size(bootstrap_regions.memories));
    reserved.reset(bootstrap_regions.reserved, std::size(bootstrap_regions.reserved));
    ustl::mem::construct_at(&memblock, ustl::move(memories), ustl::move(reserved));

    for (auto [base, size, flags] : PRESET_REGIONS) {
        memblock.add(base, size, flags.type());
    }

    reserved_list = &memblock.reserved_list();
    memories_list = &memblock.memories_list();

    action.reserve(100);
    freepath.reserve(100);
}

auto MemBlockTestFixture::validate_deallocation(PhysAddr base, usize size, PhysAddr const final_allocated) -> bool
{
    bool left_side = false, right_side = false;

    // A region inside a bigger parent region, to exploit it will lead to split the bigger into three smaller.
    std::for_each(reserved_list->begin(), reserved_list->end(), [&, base, size] (Region &region) {
        left_side |= region.end() == base;
        right_side |= base + size == region.base;
    });

    if (left_side && right_side) {
        return true;
    }

    // A region's neighbors was reclaimed so that the above can't detect them. 
    std::for_each(freepath.begin(), freepath.end(), [&] (Action const &action) {
        left_side |= action.base + action.size == base;
        right_side |= action.base == base + size;
    });

    if (left_side && right_side) {
        return true;
    }

    // The start/end of a region is the start/end of root region it belong to.
    auto memories_list = memblock.memories_list();
    for (auto &region : memories_list) {
        left_side |= base == region.base;
        right_side |= base + size == region.end();
    }

    // A randomized block may be without neighbors.
    left_side |= std::all_of(action.begin(), action.end(), [&] (Action const &action) {
        return base != action.base + action.size;
    });

    right_side |= std::all_of(action.begin(), action.end(), [&] (Action const &action) { 
        return base + size != action.base;
    });

    return left_side && (right_side || base == final_allocated);
}

// TEST_F(MemBlockTestFixture, Trim) {
//     // memblock.trim(PAGE_SIZE);
//     // IterationContext context{&memblock, RegionType::Normal};
//     // while (auto region = memblock.iterate(context)) {
//     //     ASSERT_TRUE(ustl::mem::is_aligned(region->base, PAGE_SIZE));
//     //     ASSERT_TRUE(ustl::mem::is_aligned(region->size, PAGE_SIZE));
//     // }
// }

TEST_F(MemBlockTestFixture, FixedSizeBlockAllocation) {
    memblock.trim(PAGE_SIZE);
    for (auto i = 0; i < action.capacity(); ++i) {
        auto space = memblock.allocate_bounded(PAGE_SIZE, PAGE_SIZE, 0, ustl::NumericLimits<PhysAddr>::max());
        DEBUG_ASSERT(space);

        action.emplace_back(space, PAGE_SIZE);
        ASSERT_TRUE(ustl::mem::is_aligned(space, PAGE_SIZE)) 
            << "At " << i + 1 << " times\n";
        ASSERT_TRUE(std::any_of(reserved_list->begin(), reserved_list->end(), [space] (Region &region) {
            return region.base <= space && region.size >= PAGE_SIZE;
        })) << "At " << i + 1 << " times\n";
    }

    PhysAddr final_allocated = 0;
    for (auto i : action) {
        final_allocated = std::max(final_allocated, i.base);
    }
    while (!action.empty()) {
        auto const i = random_integer(0, action.size() - 1);
        auto const [space, size] = action[i];
        action.erase(action.begin() + i);

        memblock.deallocate(space, PAGE_SIZE);
        ASSERT_TRUE(validate_deallocation(space, PAGE_SIZE, final_allocated))
        << "Base: " << space << ", Size: " << PAGE_SIZE << ", Retail: " << action.size();
        freepath.emplace_back(space, size);
    }
}

template<int N>
struct Bit
{  static constexpr usize VALUE = 1 << N;  };

template<int A, int B, int... I>
struct GenBits: GenBits<A, B - 1, B, I...> 
{};

template<int A, int... I>
struct GenBits<A, A, I...> 
{  static constexpr usize VALUE[] = { Bit<A>::VALUE, Bit<I>::VALUE... };  };

TEST_F(MemBlockTestFixture, Fixed) {
    PhysAddr const sizes[] = {
        64, 128, 16, 256, 2048, 32, 256, 128,
        // 256, 64, 8, 8, 8, 32, 32, 32, 32, 32, 32, 32
    };
    PhysAddr const bases[] = {
        4096, 4224, 4160, 4352, 6144, 4192, 4608, 4864
    };

    for (auto i = 0; i < std::size(sizes); ++i) {
        auto const size = sizes[i];
        auto space = memblock.allocate(size, size);
        ASSERT_TRUE(space);
        action.emplace_back(space, size);
    }

    PhysAddr final_allocated = 0;
    for (auto i : action) {
        final_allocated = std::max(final_allocated, i.base);
    }
    while (!action.empty()) {
        auto const i = 4;// random_integer(0, action.size() - 1);
        auto const [space, size] = action[i];
        memblock.deallocate(space, size);
        if (!validate_deallocation(space, size, final_allocated)) {
            ASSERT_TRUE(validate_deallocation(space, size, final_allocated))
            << "Space: " << space << ", Size: " << size << ", Retail: " << action.size();
        }
        freepath.emplace_back(space, size);
        action.erase(action.begin() + i);
    }
}

TEST_F(MemBlockTestFixture, RandomSizeBlockAllocation) {
    // Must reserve 1/3 space to avoid growing.
    auto const nr_actions = memories_list->capacity() / 3 * 2;
    auto &map = GenBits<3, 12>::VALUE;
    auto const n = std::size(map);

    PhysAddr last_size = 0;
    for (auto i = 0; i < nr_actions; ++i) {
        auto const size = map[random_integer(0, n - 1)];
        if (size == last_size) {
            ASSERT_TRUE(std::none_of(action.begin(), action.end(), [] (Action const &action) {
                return action.base == 0;
            }));
        }
        auto space = memblock.allocate(size, size);
        ASSERT_TRUE(space) << "Never be NULL";
        auto base = space;
        bool no_duplicated = std::none_of(action.begin(), action.end(), [base] (Action const &action) {
            return action.base == base;
        });
        ASSERT_TRUE(no_duplicated) << "Re-allocated item";
        action.emplace_back(base, size);
        last_size = size;
    }

    ASSERT_TRUE(std::none_of(action.begin(), action.end(), [] (Action const &action) {
        return action.size > 4096;
    }));

    PhysAddr final_allocated = 0;
    for (auto i : action) {
        final_allocated = std::max(final_allocated, i.base);
    }
    while (!action.empty()) {
        auto const i = random_integer(0, action.size() - 1);
        auto const [space, size] = action[i];
        memblock.deallocate(space, size);
        if (!validate_deallocation(space, size, final_allocated)) {
            ASSERT_TRUE(validate_deallocation(space, size, final_allocated))
            << "Space: " << space << ", Size: " << size << ", Retail: " << action.size();
        }
        freepath.emplace_back(space, size);
        action.erase(action.begin() + i);
    }
}

TEST_F(MemBlockTestFixture, Iterator) {

}

// TODO(SmallHuaZi) Continue to compelete this tests
TEST_F(MemBlockTestFixture, GrowSpace) {

}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
