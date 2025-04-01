#include "legacy-boot.hpp"
#include "multiboot2.hpp"

#include <ours/phys/init.hpp>
#include <ours/phys/print.hpp>
#include <ours/phys/arch-bootmem.hpp>

#include <ustl/mem/object.hpp>
#include <bootmem/memblock.hpp>

namespace ours::phys {
    static auto parse_memory_map(usize base, usize size) -> void
    {
        auto mem = global_bootmem();
        auto const mmap = reinterpret_cast<MultibootTagMmap *>(base);
        auto const end = base + size - 1;
        auto entry = mmap->entries;

        char const *get_name[MultibootMmapEntry::MaxNumType] = {
            [MultibootMmapEntry::Available] = "Available",
            [MultibootMmapEntry::Reserved] = "Reserved",
            [MultibootMmapEntry::AcpiReclaimable] = "AcpiReclaimable",
            [MultibootMmapEntry::Nvs] = "AcpiNvs",
            [MultibootMmapEntry::BadRam ] = "Bad",
        };

        // TODO(SmallHuaZi): This is a workaround, we should parse the video memory parameter to get the video memory range. 
        // and add it to the BootMem.
        mem->add(0xB0000, 0x10000, bootmem::RegionType::ReservedAndNoInit);
        mem->protect(0xB0000, 0x10000);

        println("{:16} | {:18} | {}", "Type", "Base", "Size");
        while (usize(entry) <= end) {
            println("{:16} | 0x{:<16X} | 0x{:<X}", get_name[entry->type], entry->addr, entry->len);
            switch (entry->type) {
                case MultibootMmapEntry::BadRam:
                case MultibootMmapEntry::Reserved:
                    mem->protect(entry->addr, entry->len);
                    mem->add(entry->addr, entry->len, bootmem::RegionType::ReservedAndNoInit);
                    break;
                default:
                    mem->add(entry->addr, entry->len, bootmem::RegionType::Normal);
                    break;
            }

            entry = reinterpret_cast<MultibootMemoryMap *>(reinterpret_cast<u8 *>(entry) + mmap->entry_size);
        }
    }

    auto LegacyBoot::parse_params(usize param) -> void
    {
        println("Multiboot info size {}", *reinterpret_cast<usize *>(param));
        auto tag = reinterpret_cast<MultibootTag *>(param + 8);
        while (tag->type != MULTIBOOT_TAG_TYPE_END) {
            tag = reinterpret_cast<MultibootTag *>((u8 *) tag + ((tag->size + 7) & ~7));
            switch (tag->type) {
                case MULTIBOOT_TAG_TYPE_VBE: {
                    auto vbe = reinterpret_cast<MultibootTagVbe *>(tag);
                    println("VBE mode: 0x{:X}", vbe->vbe_mode);
                    break;
                }
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
                case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
                    acpi_rsdp = (usize)reinterpret_cast<MultibootTagOldAcpi *>(tag)->rsdp;
                    acpi_version = 1;
                    println("ACPI-V1, RSDP={}", acpi_rsdp);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                    acpi_rsdp = (usize)reinterpret_cast<MultibootTagNewAcpi *>(tag)->rsdp;
                    acpi_version = 2;
                    println("ACPI-V2, RSDP={}", acpi_rsdp);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_MODULE: {
                    auto mod = reinterpret_cast<MultibootTagModule *>(tag);
                    println("`OBI` has been load at 0x{:X}", mod->mod_start);
                    auto start = reinterpret_cast<u8 *>(mod->mod_start);
                    ustl::mem::construct_at(&ramdisk_, start, mod->mod_end - mod->mod_start);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR: {
                    println("", static_cast<PhysAddr>(tag->size));
                    break;
                }
            }
        }
    }

} // namespace ours::phys