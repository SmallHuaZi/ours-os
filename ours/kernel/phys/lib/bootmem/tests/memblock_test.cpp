#include <bootmem/memblock.hpp>
#include <gtest/gtest.h>
#include <vector>
#include <random>
#include <format>

using namespace bootmem;

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

    MemBlock memblock;
    std::vector<PhysAddr> holding;
    std::vector<PhysAddr> freepath;

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
    memblock.init(bootstrap_regions.total, std::size(bootstrap_regions.total));
    for (auto [base, size, flags] : PRESET_REGIONS) {
        memblock.add(base, size, flags.get<0>());
    }

    reserved_list = &memblock.reserved_list();
    memories_list = &memblock.memories_list();

    holding.reserve(100);
    freepath.reserve(100);
}

auto MemBlockTestFixture::validate_deallocation(PhysAddr base, usize size, PhysAddr const final_allocated) -> bool
{
    bool matched = std::any_of(reserved_list->begin(), reserved_list->end(), [base] (Region &region) {
        return region.end() == base || base + PAGE_SIZE == region.base;
    });

    bool left_reclaimed_or_miss = false, right_reclaimed_or_miss = false;
    if (!matched) {
        std::for_each(freepath.begin(), freepath.end(), [&] (PhysAddr addr) {
            left_reclaimed_or_miss |= addr == base - PAGE_SIZE;
            right_reclaimed_or_miss |= addr == base + PAGE_SIZE;
        });

        // The dawn frontier.
        if (!left_reclaimed_or_miss || !right_reclaimed_or_miss) {
            auto memories_list = memblock.memories_list();
            for (auto &region : memories_list) {
                left_reclaimed_or_miss |= base == region.base;
                right_reclaimed_or_miss |= base + PAGE_SIZE == region.end();
            }

        }

        matched = left_reclaimed_or_miss && (right_reclaimed_or_miss || base == final_allocated);
    }

    return matched;
}

TEST_F(MemBlockTestFixture, Trim) {
    memblock.trim(PAGE_SIZE);
    IterationContext context{&memblock, RegionType::Normal};
    while (auto region = memblock.iterate(context)) {
        ASSERT_TRUE(ustl::mem::is_aligned(region->base, PAGE_SIZE));
        ASSERT_TRUE(ustl::mem::is_aligned(region->size, PAGE_SIZE));
    }
}

TEST_F(MemBlockTestFixture, FixedSizeBlockAllocation) {
    memblock.trim(PAGE_SIZE);
    for (auto i = 0; i < holding.capacity(); ++i) {
        auto space = memblock.allocate_bounded(PAGE_SIZE, PAGE_SIZE, 0, ustl::NumericLimits<PhysAddr>::max());
        holding.push_back(space);

        ASSERT_TRUE(ustl::mem::is_aligned(space, PAGE_SIZE)) 
            << "At " << i + 1 << " times\n";
        ASSERT_TRUE(std::any_of(reserved_list->begin(), reserved_list->end(), [space] (Region &region) {
            return region.base <= space && region.size >= PAGE_SIZE;
        })) << "At " << i + 1 << " times\n";
    }

    PhysAddr const final_allocated = holding.back();
    while (!holding.empty()) {
        auto const i = random_integer(0, holding.size());
        auto const space = holding[i];
        holding.erase(holding.begin() + i);
        freepath.push_back(space);

        memblock.deallocate(space, 4096);
        ASSERT_TRUE(validate_deallocation(space, PAGE_SIZE, final_allocated));
    }
}

template<int N>
struct Bit
{  static constexpr int VALUE = 1 << N;  };

template<int A, int B, int... I>
struct GenBits: GenBits<A, B - 1, B, I...> 
{};

template<int A, int... I>
struct GenBits<A, A, I...> 
{  static constexpr int VALUE[] = { Bit<A>::VALUE, Bit<I>::VALUE... };  };

TEST_F(MemBlockTestFixture, RandomSizeBlockAllocation) {
    // Must reserve 1/3 space to avoid growing.
    auto const nr_actions = memories_list->capacity() / 3 * 2;
    auto &map = GenBits<3, 12>::VALUE;
    auto const n = std::size(map);

    for (auto i = 0; i < nr_actions; ++i) {
        auto const size = map[random_integer(0, n)];
        memblock.allocate(size, size);
    }

    PhysAddr const final_allocated = holding.back();
    while (!holding.empty()) {
        auto const i = random_integer(0, holding.size());
        auto const space = holding[i];
        holding.erase(holding.begin() + i);
        freepath.push_back(space);

        memblock.deallocate(space, 4096);
        ASSERT_TRUE(validate_deallocation(space, PAGE_SIZE, final_allocated));
    }
}

// TODO(SmallHuaZi) Continue to compelete this tests
TEST_F(MemBlockTestFixture, GrowSpace) {

}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
