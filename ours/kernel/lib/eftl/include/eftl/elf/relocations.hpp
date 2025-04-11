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
#ifndef EFTL_ELF_RELOCATIONS_HPP
#define EFTL_ELF_RELOCATIONS_HPP 1

#include <eftl/elf/observable.hpp>
#include <eftl/elf/dynamic.hpp>

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/bit.hpp>
#include <ustl/variant.hpp>
#include <ustl/views/span.hpp>
#include <ustl/algorithms/mod.hpp>

namespace eftl::elf {
    template <typename ElfTraits>
    struct View;

    template <typename ElfTraits>
    struct Relocations: Observable <ElfTraits, Relocations<ElfTraits>> {
        typedef Relocations                 Self;
        typedef View<ElfTraits>             View;
        typedef typename View::Section      Section;
        typedef typename View::Symbol       Symbol;
        typedef typename View::Addr         Addr;
        typedef typename View::Off          Off;
        typedef typename View::Rel          Rel;
        typedef typename View::Rela         Rela;

        typedef ustl::views::Span<Addr>     RelrTable;
        typedef typename View::RelTable     RelTable;
        typedef typename View::RelaTable    RelaTable;
        typedef ustl::Variant<RelTable, RelaTable>  JmprelTable;
        typedef typename View::Symbols      Symbols;

        FORCE_INLINE CXX11_CONSTEXPR
        auto relative_reltable() -> RelTable {
            return rel_table_.subspan(0, rel_count_);
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto relative_relatable() -> RelaTable {
            return rela_table_.subspan(0, rela_count_);
        }

        template <typename Visitor>
        auto visit_relative(Visitor &&visitor) -> bool {
            // This visitor was made to help us handle the RELR relocations.
            auto make_table_visitor = [] (auto visit) {
                return [visit] (auto table) { 
                    return ustl::algorithms::all_of(table.begin(), table.end(), visit);
                };
            };

            auto visit_all_rel = make_table_visitor([&visitor] (Rel &rel) {
                return visitor(usize(rel.r_offset));
            });

            auto visit_all_rela = make_table_visitor([&visitor] (Rela &rel) {
                return visitor(usize(rel.r_offset), usize(rel.r_addend));
            });

            return visit_all_rel(relative_reltable()) &&
                   visit_all_rela(relative_relatable()) &&
                   visit_relr_table(visitor);
        }

        template <typename Visitor>
        auto visit_symbolic(Visitor &&visitor) -> bool {

        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_rel(RelTable rel, usize count) -> Self & {
            rel_table_ = rel;
            rel_count_ = ustl::algorithms::min(count, rel.size());
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_rela(RelaTable rela, usize count) -> Self & {
            rela_table_ = rela;
            rela_count_ = ustl::algorithms::min(count, rela_table_.size());
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_relr(RelrTable relr) -> Self & {
            relr_table_ = relr;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_jmprel(JmprelTable jmprel) -> Self & {
            jmprel_table_ = jmprel;
            return *this;
        }

    private:
        template <typename Visitor>
        CXX11_CONSTEXPR 
        auto visit_relr_table(Visitor &&visitor) -> bool {
            // In the .relr.dyn section,
            //  1) An even entry indicates a location which needs a relocation 
            //     and sets up where for subsequent odd entries.
            //  2) An odd entry indicates a bitmap encoding up to 63 locations
            //     following where.
            //  3) Odd entries can be chained.

            Addr r_offset = 0;
            for (auto &entry : relr_table_) {
                if ((entry & 1) == 0) {
                    if (!visitor(usize(entry))) {
                        return false;
                    }
                    continue;
                }

                Addr bitmap = entry >> 1;
                Addr bitmap_offset = r_offset;
                r_offset += (ustl::NumericLimits<Addr>::DIGITS - 1) * sizeof(Addr);
                while (bitmap != 0) {
                    i32 skip = ustl::countr_zero(bitmap) + 1;
                    bitmap >>= skip;
                    bitmap_offset += skip * sizeof(Addr);
                    if (!visitor(bitmap_offset)) {
                        return false;
                    }
                }
            }

            return true;
        }

        usize rel_count_;
        RelTable  rel_table_;
        usize rela_count_;
        RelaTable rela_table_;
        RelrTable relr_table_;
        JmprelTable jmprel_table_;
    };

    typedef ustl::TypeMap<
        ustl::TypeMapEntry<DynTagMatcher<DynTag::Rel>, ustl::traits::IntConstant<0>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::RelSz>, ustl::traits::IntConstant<1>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::RelCount>, ustl::traits::IntConstant<2>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::Rela>, ustl::traits::IntConstant<3>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::RelaSz>, ustl::traits::IntConstant<4>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::RelaCount>, ustl::traits::IntConstant<5>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::Relr>, ustl::traits::IntConstant<6>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::RelrSz>, ustl::traits::IntConstant<7>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::PltRel>, ustl::traits::IntConstant<8>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::PltRelSz>, ustl::traits::IntConstant<9>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::JmpRel>, ustl::traits::IntConstant<10>>
    > RelTagToIndexMap;

    template <typename ElfTraits>
    struct RelocationObserver;

    template <typename ElfTraits>
    using RelocationObserverBase = DynInfoObserver<RelocationObserver<ElfTraits>, Relocations<ElfTraits>, 
                                    ElfTraits, RelTagToIndexMap,
                                    DynTag::Rel, DynTag::RelSz, DynTag::Rela, DynTag::RelaSz, 
                                    DynTag::Relr,DynTag::RelrSz,DynTag::PltRel, DynTag::PltRelSz,
                                    DynTag::JmpRel, DynTag::RelCount, DynTag::RelaCount, DynTag::RelEnt,
                                    DynTag::RelaEnt, DynTag::RelrEnt>;

    template <typename ElfTraits>
    struct RelocationObserver: public RelocationObserverBase<ElfTraits> {
        typedef RelocationObserverBase<ElfTraits>   Base;
        typedef typename Base::TagIndexMap  TagIndexMap;
        typedef typename Base::InfoType     InfoType;
        typedef typename ElfTraits::Addr    Addr;
        typedef typename ElfTraits::Rel     Rel;
        typedef typename ElfTraits::Rela    Rela;
        typedef Addr    Relr;

        using Base::Base;
        using Base::at;
        using Base::write_back;
        using Base::observe;

        template <typename Memory, DynTag Tag>
        FORCE_INLINE CXX11_CONSTEXPR 
        auto observe(Memory &&memory, Addr val, DynTagMatcher<Tag> tag) -> bool {
            if CXX11_CONSTEXPR (Tag == DynTag::RelEnt) {
                return val == sizeof(Rel);
            } else if CXX11_CONSTEXPR (Tag == DynTag::RelaEnt) {
                return val == sizeof(Rela);
            } else if CXX17_CONSTEXPR (Tag == DynTag::RelrEnt) {
                return val == sizeof(Relr);
            } else {
                return Base::observe(memory, val, tag);
            }
        }

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR 
        auto finish(Memory &&memory) -> bool {
            // The tag `PltRel` indicates which format the jmprel table uses.
            if (Base::template at<DynTag::PltRel>() == Addr(DynTag::Rel)) {
                if (!Base::template write_back<Rel, &InfoType::set_jmprel, DynTag::JmpRel, DynTag::PltRelSz>(memory)) {
                    return false;
                }
            } else if (Base::template at<DynTag::PltRel>() == Addr(DynTag::Rela)) {
                if (!Base::template write_back<Rela, &InfoType::set_jmprel, DynTag::JmpRel, DynTag::PltRel>(memory)) {
                    return false;
                }
            }

            return {
                Base::template write_back<Rel, &InfoType::set_rel, DynTag::Rel, DynTag::RelSz, DynTag::RelCount>(memory) &&
                Base::template write_back<Rela, &InfoType::set_rela, DynTag::Rela, DynTag::RelaSz, DynTag::RelaCount>(memory) &&
                Base::template write_back<Relr, &InfoType::set_relr, DynTag::Relr, DynTag::RelrSz>(memory) 
            };
        }
    };

    template <typename ElfTraits>
    struct InitFiniInfo: Observable <ElfTraits, InitFiniInfo<ElfTraits>> {
        typedef InitFiniInfo                Self;
        typedef typename ElfTraits::Addr    Addr;
        typedef ustl::views::Span<Addr>     InitFiniArray;

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_array(InitFiniArray array) -> Self & {
            array_ = array;
            return *this;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_legacy(Addr val) -> Self & {
            legacy_ = val;
            return *this;
        }

        template <typename Visitor>
        FORCE_INLINE CXX11_CONSTEXPR
        auto visit(Visitor &&visitor) -> bool {
            for (auto &item : array_) {
                if (!visitor(item)) {
                    return false;
                }
            }

            return true;
        }

        Addr legacy_;
        InitFiniArray array_;
    };

    template <typename ElfTraits, DynTag Array, DynTag ArraySz, DynTag Legacy>
    class DynInitFiniObserver;

    typedef ustl::TypeMap<
        ustl::TypeMapEntry<DynTagMatcher<DynTag::InitArray>, ustl::traits::IntConstant<0>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::InitArraySz>, ustl::traits::IntConstant<1>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::FiniArray>, ustl::traits::IntConstant<0>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::FiniArraySz>, ustl::traits::IntConstant<1>>
    > InitFiniTagToIndexMap;

    // This is just a shorthand to avoid repeating the long list of parameters.
    template <typename ElfTraits, DynTag Array, DynTag ArraySz, DynTag Legacy>
    using DynInitFiniObserverBase = DynInfoObserver<DynInitFiniObserver<ElfTraits, Array, ArraySz, Legacy>, 
                                                    InitFiniInfo<ElfTraits>, ElfTraits,InitFiniTagToIndexMap,
                                                    Array, ArraySz, Legacy>;

    template <typename ElfTraits, DynTag Array, DynTag ArraySz, DynTag Legacy>
    struct DynInitFiniObserver: public DynInitFiniObserverBase<ElfTraits, Array, ArraySz, Legacy> {
        typedef DynInitFiniObserverBase<ElfTraits, Array, ArraySz, Legacy>    Base;
        typedef InitFiniInfo<ElfTraits>         Info;
        typedef typename ElfTraits::Addr        Addr;
        typedef typename Base::InfoType         InfoType;

        using Base::Base;
        using Base::observe;

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR 
        auto observe(Memory& memory, Addr val, DynTagMatcher<Legacy>) -> bool {
            this->info().set_legacy(val);
            return true;
        }

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR 
        auto finish(Memory &&memory) -> bool {
            return Base::template write_back<Addr, &InfoType::set_array, Array, ArraySz>(memory);
        }
    };

    template <typename ElfTraits>
    using DynInitObserver = DynInitFiniObserver<ElfTraits, DynTag::InitArray, DynTag::InitArraySz, DynTag::Init>;

    template <typename ElfTraits>
    using DynFiniObserver = DynInitFiniObserver<ElfTraits, DynTag::FiniArray, DynTag::FiniArraySz, DynTag::Fini>;

    template <typename ElfTraits>
    struct StrTableInfo: public Observable <ElfTraits, StrTableInfo<ElfTraits>> {
        typedef StrTableInfo                Self;
        typedef typename ElfTraits::Addr    Addr;
        typedef ustl::views::Span<Addr>     StrTable;

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_str_table(StrTable array) -> Self & {
            strtable_ = array;
            return *this;
        }

        template <typename Visitor>
        FORCE_INLINE CXX11_CONSTEXPR
        auto visit(Visitor &&visitor) -> bool {
            for (auto &item : strtable_) {
                if (!visitor(item)) {
                    return false;
                }
            }
            return true;
        }

        StrTable strtable_;
    };

    typedef ustl::TypeMap<
        ustl::TypeMapEntry<DynTagMatcher<DynTag::StrTab>, ustl::traits::IntConstant<0>>,
        ustl::TypeMapEntry<DynTagMatcher<DynTag::StrSz>, ustl::traits::IntConstant<1>>
    > DynStrTagToIndexMap;

    template <typename ElfTraits>
    struct DynStrObserver;

    template <typename ElfTraits>
    using DynStrObserverBase = DynInfoObserver<DynStrObserver<ElfTraits>, StrTableInfo<ElfTraits>, 
                                               ElfTraits, DynStrTagToIndexMap, 
                                               DynTag::StrTab, DynTag::StrSz>;

    template <typename ElfTraits>
    struct DynStrObserver: public DynStrObserverBase<ElfTraits> {
        typedef DynStrObserverBase<ElfTraits>   Base;
        typedef StrTableInfo<ElfTraits>         Info;
        typedef typename ElfTraits::Addr        Addr;
        typedef typename Base::InfoType         InfoType;

        template <typename Memory>
        FORCE_INLINE CXX11_CONSTEXPR 
        auto finish(Memory &&memory) -> bool {
            return Base::template write_back<Addr, &InfoType::set_str_table, DynTag::StrTab, DynTag::StrSz>(memory);
        }
    };

} // namespace eftl::elf

#endif // #ifndef EFTL_ELF_RELOCATIONS_HPP