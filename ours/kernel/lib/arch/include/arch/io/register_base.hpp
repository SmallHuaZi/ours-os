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
#ifndef ARCH_IO_REGISTER_BASE_HPP
#define ARCH_IO_REGISTER_BASE_HPP 1

#include <arch/types.hpp>
#include <ustl/bit.hpp>
#include <ustl/traits/is_same.hpp>
#include <ustl/traits/is_base_of.hpp>
#include <ustl/traits/is_invocable.hpp>
#include <ustl/util/visit.hpp>

namespace arch::io {
    /// struct DummyRegister
    /// { details... };
    ///
    /// struct DummyIo {
    ///     auto write_to(usize addr, )
    /// };
    ///
    /// struct IoDispatcher
    /// {  typedef DummyIo  Type;  }
    template <typename Register>
    struct IoDispatcher;

    template <typename Derived, typename IntType>
    class RegisterBase {
        typedef Derived    Self;
        static_assert(ustl::traits::IsBaseOfV<Self, Derived>, "Derived must be a subclass of this");
    public:
        typedef IntType             ValueType;
        typedef ValueType *         PtrMut;
        typedef ValueType const *   Ptr;

        FORCE_INLINE CXX11_CONSTEXPR
        auto addr() const -> u32 {
            return addr_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_addr(u32 addr) -> void {
            addr_ = addr;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto value() -> ValueType & {
            return value_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto value() const -> ValueType const & {
            return value_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_value(ValueType value) ->  Self & {
            value_ = value;
            return *static_cast<Self *>(this);
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        auto read_from(T *reg_io) -> Self & {
            auto visitor = [this] (auto&& io) { 
                value_ = io.template read<ValueType>(addr_);
            };
            ustl::visit(visitor, *reg_io);
            return *static_cast<Self *>(this);
        }

        template <typename T>
        FORCE_INLINE CXX11_CONSTEXPR
        auto write_to(T *reg_io) -> Self & {
            auto visitor = [this] (auto&& io) { 
                io.write(, addr_);
            };
            ustl::visit(visitor, *reg_io);
            return *static_cast<Self *>(this);
        }

        template <typename F>
        FORCE_INLINE CXX11_CONSTEXPR
        auto for_each_field(F &&f) const -> void {
        }

    private:
        ValueType value_;
        u32 addr_;
    };

} // namespace arch::io

#endif // #ifndef ARCH_IO_REGISTER_BASE_HPP