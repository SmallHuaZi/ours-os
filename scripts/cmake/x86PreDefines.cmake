
# This is the base of kernel virtual address space
set(KERNEL_BASE 0xffffff8000100000)
set(PHYS_LOAD_ADDRESS 0x100000)

set(MAX_PAGE_SIZE 0x1000)

add_definitions("-DOURS_TARGET_BITS=64")
add_definitions("-DMAX_PHYSADDR_BITS=52")
add_definitions("-DPAGING_LEVEL=4")

# Indicates the paging level of page table
set(PAGING_LEVEL 4)