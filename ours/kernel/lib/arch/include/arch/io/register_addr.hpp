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
#ifndef ARCH_IO_REGISTER_ADDR_HPP
#define ARCH_IO_REGISTER_ADDR_HPP 1

#include <arch/types.hpp>
#include <arch/io/register_base.hpp>

namespace arch::io {
    template <typename RegType>
    class RegisterAddr {
      public:
        typedef RegType::ValueType      ValueType;

        RegisterAddr(u32 reg_addr)
            : reg_addr_(reg_addr) {}

        static_assert(ustl::traits::IsBaseOfV<RegisterBase<RegType, typename RegType::ValueType>, RegType> ||
                      ustl::traits::IsBaseOfV<RegisterBase<RegType, typename RegType::ValueType>, RegType>,
                      "Parameter of RegisterAddr<> should derive from RegisterBase");

        // Instantiate a RegisterBase using the value of the register read from MMIO.
        template <typename T>
        auto read_from(T *reg_io) const -> RegType {
            RegType reg;
            reg.set_reg_addr(reg_addr_);
            reg.read_from(reg_io);
            return reg;
        }

        // Instantiate a RegisterBase using the given value for the register.
        auto from_value(ValueType value) const -> RegType {
            RegType reg;
            reg.set_reg_addr(reg_addr_);
            reg.set_reg_value(value);
            return reg;
        }

        auto addr() const -> u32 {
            return reg_addr_;
        }

      private:
        const u32 reg_addr_;
    };
} // namespace arch::io

#endif // #ifndef ARCH_IO_REGISTER_ADDR_HPP