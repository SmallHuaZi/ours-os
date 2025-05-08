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
#ifndef ARCH_X86_TLB_HPP
#define ARCH_X86_TLB_HPP 1

#include <arch/types.hpp>
#include <arch/system.hpp>

namespace arch {
    FORCE_INLINE
    static auto x86_invlpg(VirtAddr address) -> void {
        asm volatile("invlpg %0" ::"m"(address) : "memory");
    }

    /// Public Interface.
    FORCE_INLINE
    static auto tlb_invalidate_at(VirtAddr address) -> void {
        x86_invlpg(address);
    }

    /// Public Interface. A legacy method.
    /// Read and write CR3 register.
    FORCE_INLINE
    static auto tlb_invalidate_all() -> void {
        Cr3::read().write();
    }

    enum class InvPcidCmd: u64 {
        /// The logical processor invalidates mappings—except global translations—for the linear address and PCID specified.
        Address,

        /// The logical processor invalidates all mappings—except global translations—associated with the PCID.
        Single,

        /// The logical processor invalidates all mappings—including global translations—associated with any PCID.
        All,

        /// The logical processor invalidates all mappings—except global translations—associated with any PCID.
        AllExceptGlobal,
    };

    typedef u16     Pcid;

    CXX11_CONSTEXPR
    static Pcid const kInvalidApicId = -1;

    struct InvPcidDesc {
        u64 pcid;
        u64 addr;
    };

    /// Requires CPUID.(EAX=07H, ECX=0H):EBX.INVPCID to be 1.
    FORCE_INLINE
    static auto x86_invpcid(InvPcidDesc const &request, InvPcidCmd cmd) -> void {
        asm ("invpcid %0, %1" :: "m"(request), "r"(cmd) : "memory");
    }

    FORCE_INLINE
    static auto x86_invpcid_at(u16 pcid, usize address) -> void {
        x86_invpcid({pcid, address}, InvPcidCmd::Single);
    }

    FORCE_INLINE
    static auto x86_invpcid_single(u16 pcid) -> void {
        x86_invpcid({pcid, 0}, InvPcidCmd::Single);
    }

    FORCE_INLINE
    static auto x86_invpcid_all() -> void {
        x86_invpcid({0, 0}, InvPcidCmd::All);
    }

    FORCE_INLINE
    static auto x86_invpcid_all_without_global_pages() -> void {
        x86_invpcid({0, 0}, InvPcidCmd::AllExceptGlobal);
    }

} // namespace arch

#endif // #ifndef ARCH_X86_TLB_HPP