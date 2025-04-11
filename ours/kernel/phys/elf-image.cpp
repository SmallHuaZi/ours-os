#include <ours/phys/kernel-image.hpp>

#include <eftl/elf/dynamic.hpp>
#include <eftl/elf/link.hpp>

namespace ours::phys {
    auto ElfImage::init(ustl::views::Span<u8> file) -> Status {
        auto maybe_view = ElfView::from_raw(file.data(), file.size());
        if (maybe_view.is_err()) {
            panic("Failed to load kernel ELF image, cause: {}", to_string(maybe_view.unwrap_err()));
        }
        view_ = *maybe_view;

        ElfView::Segment dynamic;
        auto result = decode_segments(
            view_.binary(), 
            view_.segments(), 
            loadinfo_.make_observer(),
            dynamic.make_observer<eftl::elf::DynSegObserver>()
        );
        DEBUG_ASSERT(result, "Failed to decode segments");

        dynamics_ = view_.binary().read_and_cast<ElfView::Dyn>(dynamic.file_offset(), dynamic.file_size());

        return Status::Ok;
    }

    auto ElfImage::load_at(PhysAddr phys_addr) -> ustl::Result<PhysAddr, Status> {
        bool const result = loadinfo_.visit_segments([this, phys_addr] (auto &segment) {
            auto const addr = phys_addr + view_.virt_addr_to_offset(segment.va_start);
            if ((addr % segment.alignment) != 0) {
                return false;
            }

            ustl::algorithms::copy(
                segment.content.begin(),
                segment.content.end(),
                reinterpret_cast<std::decay_t<decltype(*segment.content.end())> *>(addr)
            );

            return true;
        });

        if (!result) {
            return ustl::err(Status::MisAlgined);
        }

        pa_load_ = phys_addr;
        return ustl::ok(pa_load_ + view_.virt_addr_to_offset(view_.entry_point()));
    }

    auto ElfImage::relocate(usize load_address) -> void {
        ustl::io::DirectMemory image(reinterpret_cast<u8 *>(pa_load_), size());

        InitInfo init_array{};
        Relocations relocations{};
        auto result = decode_dynamic(
            image,
            dynamics_, 
            relocations.make_observer<eftl::elf::RelocationObserver>(),
            init_array.make_observer<eftl::elf::DynInitObserver>()
        );
        DEBUG_ASSERT(result, "Failed to decode the dynamic segment");

        result = relocate_relative(image, relocations, load_address);
        DEBUG_ASSERT(result, "Failed to relocate kernel");

        va_load_ = load_address;
    }

    // HINT(SmallHuaZi) To fix page mappings for correct permissions may result in the split 
    // of a huge page , We do not implement it because the Aspace class now just support to 
    // map but to update.
    auto ElfImage::fix_mapping(Aspace &aspace) -> Status {
        Status status = Status::Ok;
        // loadinfo_.visit_segments([this] (auto &segment) {
        //     VirtAddr va_begin = segment.va_begin + load_offset_;
        //     VirtAddr va_end = segment.
        // });

        return status;
    }

} // namespace ours::phys