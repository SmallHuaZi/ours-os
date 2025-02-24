#include "../legacy-boot.hpp"
#include "multiboot2.hpp"

#include <ours/phys/init.hpp>

#include <bootmem/memblock.hpp>

namespace ours::phys {
    static bootmem::MemBlock MEMBLOCK;

    static auto parse_memory_map(usize base, usize size) -> void
    {
        auto const mmap = reinterpret_cast<MultibootTagMmap *>(base);
        auto const end = base + size - 1;
        auto entry = mmap->entries;

        while (usize(entry) <= end) {
            switch (entry->type) {
                case MultibootMmapEntry::Available:
                    MEMBLOCK.add(entry->addr, entry->len, bootmem::RegionType::Normal);
                    break;
                case MultibootMmapEntry::Reserved:
                    MEMBLOCK.add(entry->addr, entry->len, bootmem::RegionType::Reserved);
                    break;
            }

            entry = reinterpret_cast<MultibootMemoryMap *>(reinterpret_cast<u8 *>(entry) + mmap->entry_size);
        }
    }

    auto LegacyBoot::init_memory(usize param, Aspace *aspace) -> void
    {
        auto tag = reinterpret_cast<MultibootTag *>(param);
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
                    auto mods = reinterpret_cast<MultibootTagModule *>(tag)->mod_start;
                    break;
                }

            }
        }

        BOOTMEM = &MEMBLOCK;
    }

} // namespace ours::phys