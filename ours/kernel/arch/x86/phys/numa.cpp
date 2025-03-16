#include <acpi/numa.hpp>

// The default layout:
// |   63-48   | 47-39 (PML4) | 38-30 (PDP) | 29-21 (PD) | 20-12 (PT) | 11-0 (Offset) |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   9 bits   |    12 bits    |
//
// For PT: KERNEL_VIRT_BASE[(12 / 8) = 1]   | 63...8 |
// |   63-40   | 39-31 (PML4) | 30-22 (PDP) | 21-13 (PD) | 12-4 (PT) | 3-0 (Offset) |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   9 bits  |    4 bits    |
//
// For PD: KERNEL_VIRT_BASE[(21 / 8) = 2]   | 63...16 |
// |   63-32   | 31-23 (PML4) | 22-14 (PDP) |  13-5 (PD) | 4-0 (PT)  |
// |    ..     |    9 bits    |    9 bits   |   9 bits   |   5 bits  |
//
// For PDP: KERNEL_VIRT_BASE[(30 / 8) = 3]  | 63...24 |
// |   63-24   | 23-15 (PML4) | 14-6 (PDP)  |  5-0 (PD)  |
// |    ..     |    9 bits    |    9 bits   |   6 bits   |
//
// For PML4: KERNEL_VIRT_BASE[(39 / 8) = 4] | 63...32 |
// |   63-16   | 15-7 (PML4)  |  6-0 (PDP)  |
// |    ..     |    9 bits    |    7 bits   |
//
namespace ours::phys {
    

} // namespace ours::phys