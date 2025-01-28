#include <ours/kernel_entry.hpp>
#include <ours/mem/constant.hpp>

#include <logz4/log.hpp>
#include <multiboot/multiboot2.hpp>

using namespace multiboot2;

namespace ours {
    // Interim workaround.
    static auto parse_memory_params(usize start, usize end) -> void
    {
        constexpr char const *type_to_string[] = {
            "Status", 
            "Available", 
            "Reserved", 
            "AcpiReclaimable",
            "Nvs",
            "BadRam",
            "Status",
        };

        auto mmap = reinterpret_cast<MultibootTagMmap *>(start);
        auto entry = mmap->entries;

        while (reinterpret_cast<usize>(entry) < end) {
            // Log::debug("{ 0x%x%x, 0x%x%x, %s }\n",
            //         u32(entry->addr >> 32),
            //         u32(entry->addr & 0xffffffff),
            //         u32(entry->len >> 32),
            //         u32(entry->len & 0xffffffff),
            //         type_to_string[entry->type]
            // );
            
            mem::MemRegion const section{ usize(entry->addr), usize(entry->addr) + usize(entry->len) - 1 };
            // Log::debug("{ 0x%x, 0x%x, %s }\n", section.base_, section.end_, type_to_string[entry->type]);
            switch (entry->type) {
                case MultibootMmapEntry::Available:
                    // G_EARLY_ALLOC.insert(section);
                    break;
                case MultibootMmapEntry::Reserved:
                    // G_EARLY_ALLOC.protect(section);
                    break;
            }

            entry = reinterpret_cast<MultibootMemoryMap *>(
                reinterpret_cast<u8 *>(entry) + mmap->entry_size
            );
        }
    }

    static auto parse_acpi_rsdp() -> void
    {

    }

    auto cls() -> void;

    NO_MANGLE
    auto x86_main(u32 addr, u32 loader_magic) -> void
    {
        cls();
        if (loader_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
            log::debug("A unknown bootloader-magic number.\n");
            return;
        }

        auto const mbi_size = *reinterpret_cast<usize *>(addr);
        auto tag = reinterpret_cast<MultibootTag *>(addr + 8);
        while (tag->type != MULTIBOOT_TAG_TYPE_END) {
            tag = reinterpret_cast<MultibootTag *>((u8 *) tag + ((tag->size + 7) & ~7));
            switch (tag->type) {
                case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME: {
                    log::debug("Loader name: %s\n", (char const *)reinterpret_cast<MultibootTagString *>(tag)->string);
                    break;
                }
                case MULTIBOOT_TAG_TYPE_MMAP: {
                    parse_memory_params(usize(tag), usize(tag) + tag->size);
                    log::debug("After parse_memory_params\n");
                    break;
                }
                case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
                    auto rsdp = reinterpret_cast<MultibootTagNewAcpi *>(tag)->rsdp;
                    break;
                }
            }
        }

        // G_EARLY_ALLOC.dump();

        // constexpr mem::Layout const layout{ PAGE_SIZE, PAGE_SIZE };
        // auto *page = G_EARLY_ALLOC.allocate(layout);
        // Log::debug("Page at 0x%x\n", page);

        // // G_EARLY_ALLOC.dump();

        // G_EARLY_ALLOC.deallocate(page, layout);
        // G_EARLY_ALLOC.dump();

        start_kernel(0);
        while (1);
    }

}