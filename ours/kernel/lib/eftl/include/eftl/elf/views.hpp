/// Copyright(C) 2024 smallhuazi
///
/// This program is free software; you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published
/// by the Free Software Foundation; either version 2 of the License, or
/// (at your option) any later version.
///
/// For additional information, please refer to the following website:
/// https://opensource.org/license/gpl-2-0
///
#ifndef EFTL_ELF_VIEWS_HPP
#define EFTL_ELF_VIEWS_HPP 1

#include <eftl/elf/traits.hpp>
#include <eftl/elf/enums.hpp>
#include <eftl/elf/link.hpp>
#include <eftl/elf/load.hpp>
#include <eftl/elf/relocations.hpp>
#include <eftl/elf/dynamic.hpp>
#include <eftl/elf/segment.hpp>

#include <ustl/algorithms/minmax.hpp>
#include <ustl/io/direct_memory.hpp>
#include <ustl/views/filter_view.hpp>
#include <ustl/views/transform_view.hpp>
#include <ustl/iterator/traits.hpp>
#include <ustl/collections/static-vec.hpp>
#include <ustl/option.hpp>
#include <ustl/result.hpp>
#include <ustl/mem/align.hpp>
#include <ustl/views/span.hpp>

namespace eftl::elf {
    enum class ParseError {
        NoError,
        InternalError,
        InvalidHeader,
        InvalidSegment,
        InvalidSection,
        InvalidSymbol,
        InvalidRelocation,
    };

    template <typename ElfTraits = ElfTraitsNative>
    struct View: public ElfTraits {
        using  Self = View;
        using  Base = ElfTraits;
        using  typename Base::Addr; 
        using  typename Base::Off;
        using  typename Base::Half;
        using  typename Base::Word;
        using  typename Base::Sword;

        using  typename Base::Header;
        using  typename Base::Segment;
        using  typename Base::Section;
        using  typename Base::Symbol;
        using  typename Base::Auxv;
        using  typename Base::Dyn;
        using  typename Base::Rel;
        using  typename Base::Rela;
        using  typename Base::Verneed;
        using  typename Base::Verdef;
        using  typename Base::Vernaux;
        using  typename Base::Prstatus;
        using  typename Base::FileEntry;

        typedef ustl::views::Span<Segment>     Segments;
        typedef ustl::views::Span<Section>     Sections;
        typedef ustl::views::Span<Symbol>      Symbols;
        typedef ustl::views::Span<Dyn>         Dynamics;
        typedef ustl::views::Span<Rel>         RelTable;
        typedef ustl::views::Span<Rela>        RelaTable;

        View() = default;

        View(Self const &) = default;

        View(Self &&) = default;

        ~View() = default;

        static auto from_raw(void *raw, usize size_bytes) -> ustl::Result<View, ParseError> {
            Self view;
            view.binary_.reset(raw, size_bytes);
            if (auto result = view.init()) {
                return ustl::err(*result);
            }

            return ustl::ok(ustl::move(view));
        }

        template <typename T>
        auto write(usize pos, T value) -> void {
        }

        auto apply_relocation(isize offset) -> bool {
            ustl::io::DirectMemory io(binary_);
            Relocations<ElfTraits> relocations;
            if (!decode_dynamic(io, dynamics_, RelocationObserver<ElfTraits>(relocations))) {
                return false;
            }

            return relocate_relative(io, relocations, offset);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto virt_addr_to_offset(Addr addr) const -> usize {
            if (va_minimal_ > addr || addr > va_maximal_) {
                return ustl::NumericLimits<usize>::max();
            }
            return addr - va_minimal_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto segments() const -> Segments {
            return segments_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto sections() const -> Sections {
            return sections_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto entry_point() const -> Addr {
            return header_->e_entry;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto virtual_size() const -> usize {
            return va_maximal_ - va_minimal_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto binary() -> ustl::io::DirectMemory {
            return binary_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto data() -> u8 * {
            return binary_.address();
        }

        auto operator=(Self &&) -> Self & = default;
        auto operator=(Self const &) -> Self & = default;
    private:
        auto init() -> ustl::Option<ParseError> {
            if (binary_.size() < sizeof(Header)) {
                return ParseError::InvalidHeader;
            }
            header_ = binary_.read_at<Header>(0);

            segments_ = binary_.read_at_n<Segment>(header_->e_phoff, header_->e_phnum);
            if (segments_.empty()) {
                return ustl::some(ParseError::InvalidSegment);
            }

            sections_ = binary_.read_at_n<Section>(header_->e_shoff, header_->e_shnum);
            if (sections_.empty()) {
                return ParseError::InvalidSection;
            }

            if (!decode_segments(binary_, segments_, VaSpaceObserver<ElfTraits>(va_minimal_, va_maximal_))) {
                return ParseError::InternalError;
            }

            return ustl::none();
        }

        ustl::io::DirectMemory binary_;
        Header *header_;
        Segments segments_;
        Sections sections_;
        Dynamics dynamics_;
        Addr va_maximal_;
        Addr va_minimal_;
    };

    FORCE_INLINE CXX11_CONSTEXPR
    auto to_string(ParseError  parse_error) -> char const * {
        switch (parse_error) {
            case ParseError::NoError:
                return "No error";
            case ParseError::InvalidHeader:
                return "Invalid header";
            case ParseError::InvalidSegment:
                return "Invalid segment";
            case ParseError::InvalidSection:
                return "Invalid section";
            case ParseError::InvalidSymbol:
                return "Invalid symbol";
            case ParseError::InvalidRelocation:
                return "Invalid relocation";
            case ParseError::InternalError:
                return "Internal error";
        }
        return "Unknown ELF parse error";
    }
} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_VIEWS_HPP