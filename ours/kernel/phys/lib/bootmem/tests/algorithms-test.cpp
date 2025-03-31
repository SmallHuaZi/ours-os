#include <bootmem/algorithms.hpp>
#include <gtest/gtest.h>

using namespace bootmem;

TEST(S, S) {
    Region all_regions[] = {
        { 0x1000, 0xFFFF, RegionType::Normal, 0 },
    };

    Region used_regions[] = {
        { 0x1000, 0x100, RegionType::Normal, 0 },
        { 0x11FF, 0x100, RegionType::Normal, 0 },
        { 0x18FF, 0x100, RegionType::Normal, 0 },
        { 0x24FF, 0x100, RegionType::Normal, 0 },
        { 0x38FF, 0x100, RegionType::Normal, 0 },
        { 0x50FF, 0x100, RegionType::Normal, 0 },
        { 0x61FF, 0x100, RegionType::Normal, 0 },
        { 0x71FF, 0x100, RegionType::Normal, 0 },
    };
    auto all_iter = std::begin(all_regions);
    auto used_iter = std::begin(used_regions);

    auto region = lookup_next_free_region(
        std::begin(all_regions), std::end(all_regions), 
        std::begin(used_regions), std::end(used_regions), 
        all_iter, used_iter);
    ASSERT_TRUE(region);
    ASSERT_TRUE(region->first == 0x1100 && region->second == 0x11FF);

    region = lookup_next_free_region(
        std::begin(all_regions), std::end(all_regions), 
        std::begin(used_regions), std::end(used_regions), 
        all_iter, used_iter);
    ASSERT_TRUE(region);
}