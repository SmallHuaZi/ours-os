#include "../legacy-boot.hpp"
#include "multiboot2.hpp"

#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>

#include <ustl/mem/object.hpp>
#include <bootmem/memblock.hpp>

namespace ours::phys {
    static bootmem::MemBlock MEMBLOCK;
    static bootmem::Region BOOTSTRAP_REGIONS[200];

    static auto parse_memory_map(usize base, usize size) -> void
    {
        auto const mmap = reinterpret_cast<MultibootTagMmap *>(base);
        auto const end = base + size - 1;
        auto entry = mmap->entries;

        char const *get_name[MultibootMmapEntry::MaxNumType] = {
            [MultibootMmapEntry::Available] = "available",
            [MultibootMmapEntry::Reserved] = "reserved",
            [MultibootMmapEntry::AcpiReclaimable] = "acpi-reclaimable",
            [MultibootMmapEntry::Nvs] = "acpi-nvs",
            [MultibootMmapEntry::BadRam ] = "bad",
        };

        while (usize(entry) <= end) {
            println("{}: base={:X}, size={:X}", get_name[entry->type], entry->addr, entry->len);
            switch (entry->type) {
                case MultibootMmapEntry::AcpiReclaimable:
                    // Now i am not sure if this kind of region is reclaimable, through it was marked `*Reclaimable`.
                    break;
                case MultibootMmapEntry::Reserved:
                    MEMBLOCK.protect(entry->addr, entry->len);
                    break;
            }

            MEMBLOCK.add(entry->addr, entry->len, bootmem::RegionType::Normal);
            entry = reinterpret_cast<MultibootMemoryMap *>(reinterpret_cast<u8 *>(entry) + mmap->entry_size);
        }
    }

    auto LegacyBoot::init_memory(usize param, Aspace *aspace) -> void
    {
        ustl::mem::construct_at(&MEMBLOCK);
        MEMBLOCK.init(BOOTSTRAP_REGIONS, std::size(BOOTSTRAP_REGIONS));

        println("Multiboot info size {}", *reinterpret_cast<usize *>(param));
        auto tag = reinterpret_cast<MultibootTag *>(param + 8);
        while (tag->type != MULTIBOOT_TAG_TYPE_END) {
            tag = reinterpret_cast<MultibootTag *>((u8 *) tag + ((tag->size + 7) & ~7));
            switch (tag->type) {
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                    name_ = reinterpret_cast<MultibootTagString *>(tag)->string;
                    break;
                }
                case MULTIBOOT_TAG_TYPE_CMDLINE: {
                    clparams_ = reinterpret_cast<MultibootTagString *>(tag)->string;
                    break;
                }
                case MULTIBOOT_TAG_TYPE_MMAP: {
                    parse_memory_map(usize(tag), tag->size);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                    acpi_rsdp = (usize)reinterpret_cast<MultibootTagNewAcpi *>(tag)->rsdp;
                    break;
                }
                case MULTIBOOT_TAG_TYPE_MODULE: {
                    print("`phys-ours` has been load at {:X}", reinterpret_cast<MultibootTagLoadBaseAddr *>(tag)->load_base_addr);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
                    println("", static_cast<PhysAddr>(tag->size));
                    break;
                }
            }
        }

        BOOTMEM = &MEMBLOCK;
    }

} // namespace ours::phys