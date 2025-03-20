#include <bootmem/region.hpp>
#include <gtest/gtest.h>

using namespace bootmem;

TEST(BootMemRegion, SetAndGetFlags) {
    Region region;
    memset(&region, 0, sizeof(region));

    region.set_nid(10);
    ASSERT_EQ(region.nid(), 10);

    region.set_type(RegionType::Reserved);
    ASSERT_EQ(region.nid(), 10);
    ASSERT_EQ(region.type(), RegionType::Reserved);

    region.set_nid(22);
    ASSERT_EQ(region.type(), RegionType::Reserved);
}