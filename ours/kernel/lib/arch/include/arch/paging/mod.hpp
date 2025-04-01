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
#ifndef ARCH_PAGING_MOD_HPP
#define ARCH_PAGING_MOD_HPP 1

#include <arch/paging/paging_traits.hpp>
#include <arch/paging/visitors.hpp>

#include <ustl/array.hpp>
#include <ustl/util/pair.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/traits/invoke_result.hpp>
#include <ustl/util/index_sequence.hpp>

namespace arch::paging {
    template <typename ArchPaging>
    struct Paging: public PagingTraits<ArchPaging> {
        typedef PagingTraits<ArchPaging>            PagingTraits;
        typedef typename PagingTraits::LevelType    LevelType;

        template <LevelType Level>
        using Pte = typename PagingTraits::template Pte<Level>;

        template <LevelType Level>
        using Table = ustl::Array<Pte<Level>, PagingTraits::template kNumPtes<Level>>;

        template <typename Visitor, typename PhysToVirt>
        struct MakeVisitResult {
            template <LevelType Level>
            struct VisitResultAt {
                static_assert(ustl::traits::IsInvocableV<Visitor, Table<Level>&, Pte<Level>&, VirtAddr>);
                typedef ustl::traits::InvokeResultT<Visitor, Table<Level>&, Pte<Level>&, VirtAddr>   ResultType;
                typedef typename ResultType::Element    Type;
                static_assert(ustl::traits::IsSameV<ResultType, ustl::Option<Type>>);
            };

            template <LevelType Level>
            using VisitResultAtT = typename VisitResultAt<Level>::Type;

            typedef typename VisitResultAt<PagingTraits::kFinalLevel>::Type  ValueType;
            typedef ustl::Option<ValueType>    Type;

            template <usize... LevelIndex>
            CXX11_CONSTEXPR
            static auto value_types_coincide(ustl::IndexSequence<LevelIndex...>) -> bool {
                return (ustl::traits::IsSameV<ValueType, VisitResultAtT<PagingTraits::kAllLevels[LevelIndex]>> && ...);
            }
            static_assert(value_types_coincide(ustl::MakeIndexSequenceT<std::size(PagingTraits::kAllLevels)>()));
        };

        template <typename PhysToVirt, typename Visitor>
        using VisitResult = typename MakeVisitResult<Visitor, PhysToVirt>::Type;

        template <typename PhysToVirt>
        static auto query(PhysAddr table, PhysToVirt &&phys_to_table, VirtAddr va) -> ustl::Option<QueryResult> {
            QueryVisitor<PagingTraits> visitor{};
            if (auto result = visit_page_tables(table, phys_to_table, visitor, va)) {
                return result;
            }

            return ustl::none();
        }

        template <typename PhysToVirt, typename Allocator>
        static auto map(PhysAddr table, PhysToVirt &&phys_to_table, Allocator &&allocator, VirtAddr va, PhysAddr pa, usize n,
                        MmuFlags flags, usize page_size = PAGE_SIZE) -> ustl::Result<void, MapError> {
            MappingVisitor<PagingTraits, Allocator> visitor(allocator, va, &pa, 1, flags, n * page_size);
            while (visitor.has_more()) {
                auto error = visit_page_tables(table, phys_to_table, visitor, visitor.context_.virt_addr());
                if (error->is_err()) {
                    return *error;
                }
            }

            return ustl::ok();
        }

        // TODO(SmallHuaZi): Write the body.
        template <typename PhysToVirt, typename Allocator>
        static auto unmap(PhysAddr table, PhysToVirt &&phys_to_table, VirtAddr va, usize n) {
            // Now it is unnecessary.
            // return visit_page_tables(table, phys_to_table, visitor, va);
        }

        template <typename PhysToVirt, typename Visitor>
        static auto visit_page_tables(PhysAddr table, PhysToVirt &&p2v, Visitor &&visitor, VirtAddr addr)
            -> VisitResult<PhysToVirt, Visitor> {
            return visit_page_tables_from<PagingTraits::kPagingLevel>(table, p2v, visitor, addr);
        }

        template <LevelType Level, typename PhysToVirt, typename Visitor>
        static auto visit_page_tables_from(PhysAddr pa_table, PhysToVirt &&phys_to_table, Visitor &&visitor, VirtAddr addr) 
            -> VisitResult<PhysToVirt, Visitor> {
            auto const index = PagingTraits::virt_to_index(Level, addr);
            Table<Level> &table = *reinterpret_cast<Table<Level> *>(phys_to_table(pa_table));
            Pte<Level> &entry = table[index];
            if (auto result = visitor(table, entry, addr)) {
                return *result;
            } 
            
            if CXX17_CONSTEXPR (Level != PagingTraits::kFinalLevel) {
                return visit_page_tables_from<PagingTraits::next_level(Level)>(
                    entry.address(), 
                    phys_to_table, 
                    visitor, 
                    addr
                );
            }

            return ustl::none();
        }
    };
} // namespace arch::paging

#endif // #ifndef ARCH_PAGING_MOD_HPP