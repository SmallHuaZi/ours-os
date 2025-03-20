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
    template <typename Derived, typename SizeType>
    class RegisterBase {
        static_assert(ustl::traits::IsBaseOfV<RegisterBase, Derived>, "Derived must be the subclass of this");
    public:
        typedef Derived             Self;
        typedef SizeType            ValueType;
        typedef ValueType *         PtrMut;
        typedef ValueType const *   Ptr;

        FORCE_INLINE CXX11_CONSTEXPR
        auto reg_addr() const -> u32 {
            return addr_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_reg_addr(u32 addr) -> void {
            addr_ = addr;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto reg_value() const -> ValueType {
            return value_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto value_ptr() -> PtrMut {
            return &value_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto value_ptr() const -> Ptr {
            return &value_;
        }

        FORCE_INLINE CXX11_CONSTEXPR
        auto set_reg_value(ValueType value) ->  Self & {
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

        // Invokes print_fn(const char* buf) once for each field, including each
        // RsvdZ field, and one extra time if there are any undefined bits set.
        // The callback argument must not be accessed after the callback
        // returns.  The callback will be called once for each field with a
        // null-terminated string describing the name and contents of the field.
        //
        // Printed fields will look like: "field_name[26:8]: 0x00123 (291)"
        // The undefined bits message will look like: "unknown set bits: 0x00301000"
        //
        // WARNING: This will substantially increase code size and stack usage at the
        // call site. https://fxbug.dev/42147424 tracks investigation to improve this.
        //
        // Example use:
        // reg.Print([](const char* arg) { printf("%s\n", arg); });
        template <typename F>
        FORCE_INLINE CXX11_CONSTEXPR
        auto for_each_field(F &&f) const -> void {
        }

      private:
        u32 addr_;
        ValueType value_;
    };

} // namespace arch::io

#endif // #ifndef ARCH_IO_REGISTER_BASE_HPP