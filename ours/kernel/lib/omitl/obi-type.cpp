#include <omitl/obi-type.hpp>
#include <ustl/algorithms/compare.hpp>

using ustl::algorithms::lexicographical_compare;

namespace omitl {
    auto to_string(ObiType type) -> char const * {
        switch (type) {
            case ObiType::Container:        return "cont";
            case ObiType::KernelPackage:    return "kpkg";
            case ObiType::Ramdisk:          return "ramd";
        }

        return "unknown";
    }

    template <>
    auto from_string(char const *name) -> ObiType {
        ObiType const *str = reinterpret_cast<ObiType const *>(name);
        if (*str == ObiType::Container) {
            return ObiType::Container;
        } else if (*str == ObiType::KernelPackage) {
            return ObiType::KernelPackage;
        } else if (*str == ObiType::Ramdisk) {
            return ObiType::Ramdisk;
        }

        return ObiType::Unknown;
    }

    auto to_string(ObiExeSign type) -> char const * {
        switch (type) {
            case ObiExeSign::Raw:   return "raw";
            case ObiExeSign::Elf:   return "elf";
        }

        return "unknown";
    }

    template <>
    auto from_string(char const *name) -> ObiExeSign {
        ObiExeSign const *str = reinterpret_cast<ObiExeSign const *>(name);
        if (*str == ObiExeSign::Raw) {
            return ObiExeSign::Raw;
        } else if (*str == ObiExeSign::Elf) {
            return ObiExeSign::Elf;
        }

        return ObiExeSign::Unknown;
    }

} // namespace omitl