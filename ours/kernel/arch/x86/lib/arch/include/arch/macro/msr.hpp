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
#ifndef ARCH_MACRO_MSR_HPP
#define ARCH_MACRO_MSR_HPP 1

// MSR
#define X86_MSR_IA32_PLATFORM_ID 0x00000017 // Platform id
#define X86_MSR_IA32_APIC_BASE 0x0000001b   // APIC base physical address
#define X86_MSR_IA32_TSC_ADJUST 0x0000003b  // TSC adjust
#define X86_MSR_IA32_SPEC_CTRL 0x00000048   // Speculative Execution Controls

// MSR
#define X86_MSR_SMI_COUNT 0x00000034            // Number of SMI interrupts since boot
#define X86_MSR_IA32_PRED_CMD 0x00000049        // Indirect Branch Prediction Command
#define X86_MSR_IA32_BIOS_UPDT_TRIG 0x00000079u // Microcode Patch Loader
#define X86_MSR_IA32_BIOS_SIGN_ID 0x0000008b    // BIOS update signature
#define X86_MSR_IA32_MTRRCAP 0x000000fe         // MTRR capability
#define X86_MSR_IA32_ARCH_CAPABILITIES 0x0000010a
#define X86_ARCH_CAPABILITIES_RDCL_NO (1ull << 0)
#define X86_ARCH_CAPABILITIES_IBRS_ALL (1ull << 1)
#define X86_ARCH_CAPABILITIES_RSBA (1ull << 2)
#define X86_ARCH_CAPABILITIES_SSB_NO (1ull << 4)
#define X86_ARCH_CAPABILITIES_MDS_NO (1ull << 5)
#define X86_ARCH_CAPABILITIES_TSX_CTRL (1ull << 7)
#define X86_ARCH_CAPABILITIES_TAA_NO (1ull << 8)
#define X86_MSR_IA32_FLUSH_CMD 0x0000010b       // L1D$ Flush control
#define X86_MSR_IA32_TSX_CTRL 0x00000122        // Control to enable/disable TSX instructions
#define X86_TSX_CTRL_RTM_DISABLE (1ull << 0)    // Force all RTM instructions to abort
#define X86_TSX_CTRL_CpuNum_DISABLE (1ull << 1) // Mask RTM and HLE in CpuNum
#define X86_MSR_IA32_SYSENTER_CS 0x00000174     // SYSENTER CS
#define X86_MSR_IA32_SYSENTER_ESP 0x00000175    // SYSENTER ESP
#define X86_MSR_IA32_SYSENTER_EIP 0x00000176    // SYSENTER EIP
#define X86_MSR_IA32_MCG_CAP 0x00000179         // global machine check capability
#define X86_MSR_IA32_MCG_STATUS 0x0000017a      // global machine check status
#define X86_MSR_IA32_MISC_ENABLE 0x000001a0     // enable/disable misc processor features
#define X86_MSR_IA32_MISC_ENABLE_TURBO_DISABLE (1ull << 38)
#define X86_MSR_IA32_TEMPERATURE_TARGET 0x000001a2 // Temperature target
#define X86_MSR_IA32_ENERGY_PERF_BIAS 0x000001b0   // Energy / Performance Bias
#define X86_MSR_IA32_MTRR_PHYSBASE0 0x00000200     // MTRR PhysBase0
#define X86_MSR_IA32_MTRR_PHYSMASK0 0x00000201     // MTRR PhysMask0
#define X86_MSR_IA32_MTRR_PHYSMASK9 0x00000213     // MTRR PhysMask9
#define X86_MSR_IA32_MTRR_DEF_TYPE 0x000002ff      // MTRR default type
#define X86_MSR_IA32_MTRR_FIX64K_00000 0x00000250  // MTRR FIX64K_00000
#define X86_MSR_IA32_MTRR_FIX16K_80000 0x00000258  // MTRR FIX16K_80000
#define X86_MSR_IA32_MTRR_FIX16K_A0000 0x00000259  // MTRR FIX16K_A0000
#define X86_MSR_IA32_MTRR_FIX4K_C0000 0x00000268   // MTRR FIX4K_C0000
#define X86_MSR_IA32_MTRR_FIX4K_F8000 0x0000026f   // MTRR FIX4K_F8000
#define X86_MSR_IA32_PAT 0x00000277                // PAT
#define X86_MSR_IA32_TSC_DEADLINE 0x000006e0       // TSC deadline

#define X86_MSR_IA32_X2APIC_APICID 0x00000802      // x2APIC ID Register (R/O)
#define X86_MSR_IA32_X2APIC_VERSION 0x00000803     // x2APIC Version Register (R/O)
#define X86_MSR_IA32_X2APIC_TPR 0x00000808         // x2APIC Task Priority Register (R/W)
#define X86_MSR_IA32_X2APIC_PPR 0x0000080A         // x2APIC Processor Priority Register (R/O)
#define X86_MSR_IA32_X2APIC_EOI 0x0000080B         // x2APIC EOI Register (W/O)
#define X86_MSR_IA32_X2APIC_LDR 0x0000080D         // x2APIC Logical Destination Register (R/O)
#define X86_MSR_IA32_X2APIC_SIVR 0x0000080F        // x2APIC Spurious Interrupt Vector Register (R/W)
#define X86_MSR_IA32_X2APIC_ISR0 0x00000810        // x2APIC In-Service Register Bits 31:0 (R/O)
#define X86_MSR_IA32_X2APIC_ISR1 0x00000811        // x2APIC In-Service Register Bits 63:32 (R/O)
#define X86_MSR_IA32_X2APIC_ISR2 0x00000812        // x2APIC In-Service Register Bits 95:64 (R/O)
#define X86_MSR_IA32_X2APIC_ISR3 0x00000813        // x2APIC In-Service Register Bits 127:96 (R/O)
#define X86_MSR_IA32_X2APIC_ISR4 0x00000814        // x2APIC In-Service Register Bits 159:128 (R/O)
#define X86_MSR_IA32_X2APIC_ISR5 0x00000815        // x2APIC In-Service Register Bits 191:160 (R/O)
#define X86_MSR_IA32_X2APIC_ISR6 0x00000816        // x2APIC In-Service Register Bits 223:192 (R/O)
#define X86_MSR_IA32_X2APIC_ISR7 0x00000817        // x2APIC In-Service Register Bits 255:224 (R/O)
#define X86_MSR_IA32_X2APIC_TMR0 0x00000818        // x2APIC Trigger Mode Register Bits 31:0 (R/O)
#define X86_MSR_IA32_X2APIC_TMR1 0x00000819        // x2APIC Trigger Mode Register Bits 63:32 (R/O)
#define X86_MSR_IA32_X2APIC_TMR2 0x0000081A        // x2APIC Trigger Mode Register Bits 95:64 (R/O)
#define X86_MSR_IA32_X2APIC_TMR3 0x0000081B        // x2APIC Trigger Mode Register Bits 127:96 (R/O)
#define X86_MSR_IA32_X2APIC_TMR4 0x0000081C        // x2APIC Trigger Mode Register Bits 159:128 (R/O)
#define X86_MSR_IA32_X2APIC_TMR5 0x0000081D        // x2APIC Trigger Mode Register Bits 191:160 (R/O)
#define X86_MSR_IA32_X2APIC_TMR6 0x0000081E        // x2APIC Trigger Mode Register Bits 223:192 (R/O)
#define X86_MSR_IA32_X2APIC_TMR7 0x0000081F        // x2APIC Trigger Mode Register Bits 255:224 (R/O)
#define X86_MSR_IA32_X2APIC_IRR0 0x00000820        // x2APIC Interrupt Request Register Bits 31:0 (R/O)
#define X86_MSR_IA32_X2APIC_IRR1 0x00000821        // x2APIC Interrupt Request Register Bits 63:32 (R/O)
#define X86_MSR_IA32_X2APIC_IRR2 0x00000822        // x2APIC Interrupt Request Register Bits 95:64 (R/O)
#define X86_MSR_IA32_X2APIC_IRR3 0x00000823        // x2APIC Interrupt Request Register Bits 127:96 (R/O)
#define X86_MSR_IA32_X2APIC_IRR4 0x00000824        // x2APIC Interrupt Request Register Bits 159:128 (R/O)
#define X86_MSR_IA32_X2APIC_IRR5 0x00000825        // x2APIC Interrupt Request Register Bits 191:160 (R/O)
#define X86_MSR_IA32_X2APIC_IRR6 0x00000826        // x2APIC Interrupt Request Register Bits 223:192 (R/O)
#define X86_MSR_IA32_X2APIC_IRR7 0x00000827        // x2APIC Interrupt Request Register Bits 255:224 (R/O)
#define X86_MSR_IA32_X2APIC_ESR 0x00000828         // x2APIC Error Status Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_CMCI 0x0000082F    // x2APIC LVT Corrected Machine Check Interrupt Register (R/W)
#define X86_MSR_IA32_X2APIC_ICR 0x00000830         // x2APIC Interrupt Command Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_TIMER 0x00000832   // x2APIC LVT Timer Interrupt Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_THERMAL 0x00000833 // x2APIC LVT Thermal Sensor Interrupt Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_PMI 0x00000834     // x2APIC LVT Performance Monitor Interrupt Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_LINT0 0x00000835   // x2APIC LVT LINT0 Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_LINT1 0x00000836   // x2APIC LVT LINT1 Register (R/W)
#define X86_MSR_IA32_X2APIC_LVT_ERROR 0x00000837   // x2APIC LVT Error Register (R/W)
#define X86_MSR_IA32_X2APIC_INIT_COUNT 0x00000838  // x2APIC Initial Count Register (R/W)
#define X86_MSR_IA32_X2APIC_CUR_COUNT 0x00000839   // x2APIC Current Count Register (R/O)
#define X86_MSR_IA32_X2APIC_DIV_CONF 0x0000083E    // x2APIC Divide Configuration Register (R/W)
#define X86_MSR_IA32_X2APIC_SELF_IPI 0x0000083F    // x2APIC Self IPI Register (W/O)

#define X86_MSR_IA32_EFER 0xc0000080             // EFER
#define X86_MSR_IA32_STAR 0xc0000081             // system call address
#define X86_MSR_IA32_LSTAR 0xc0000082            // long mode call address
#define X86_MSR_IA32_CSTAR 0xc0000083            // ia32-e compat call address
#define X86_MSR_IA32_FMASK 0xc0000084            // system call flag mask
#define X86_MSR_IA32_FS_BASE 0xc0000100          // fs base address
#define X86_MSR_IA32_GS_BASE 0xc0000101          // gs base address
#define X86_MSR_IA32_KERNEL_GS_BASE 0xc0000102   // kernel gs base
#define X86_MSR_IA32_TSC_AUX 0xc0000103          // TSC aux
#define X86_MSR_IA32_PM_ENABLE 0x00000770        // enable/disable HWP
#define X86_MSR_IA32_HWP_CAPABILITIES 0x00000771 // HWP performance range enumeration
#define X86_MSR_IA32_HWP_REQUEST 0x00000774      // power manage control hints
#define X86_MSR_AMD_VIRT_SPEC_CTRL 0xc001011f    // AMD speculative execution controls
                                                 // See IA32_SPEC_CTRL

// Non-architectural MSRs
#define X86_MSR_POWER_CTL 0x000001fc               // Power Control Register
#define X86_MSR_RAPL_POWER_UNIT 0x00000606         // RAPL unit multipliers
#define X86_MSR_PKG_POWER_LIMIT 0x00000610         // Package power limits
#define X86_MSR_PKG_ENERGY_STATUS 0x00000611       // Package energy status
#define X86_MSR_PKG_POWER_INFO 0x00000614          // Package power range info
#define X86_MSR_DRAM_POWER_LIMIT 0x00000618        // DRAM RAPL power limit control
#define X86_MSR_DRAM_ENERGY_STATUS 0x00000619      // DRAM energy status
#define X86_MSR_PP0_POWER_LIMIT 0x00000638         // PP0 RAPL power limit control
#define X86_MSR_PP0_ENERGY_STATUS 0x00000639       // PP0 energy status
#define X86_MSR_PP1_POWER_LIMIT 0x00000640         // PP1 RAPL power limit control
#define X86_MSR_PP1_ENERGY_STATUS 0x00000641       // PP1 energy status
#define X86_MSR_PLATFORM_ENERGY_COUNTER 0x0000064d // Platform energy counter
#define X86_MSR_PPERF 0x0000064e                   // Productive performance count
#define X86_MSR_PERF_LIMIT_REASONS 0x0000064f      // Clipping cause register
#define X86_MSR_GFX_PERF_LIMIT_REASONS 0x000006b0  // Clipping cause register for graphics
#define X86_MSR_PLATFORM_POWER_LIMIT 0x0000065c    // Platform power limit control
#define X86_MSR_AMD_F10_DE_CFG 0xc0011029          // AMD Family 10h+ decode config
#define X86_MSR_AMD_F10_DE_CFG_LFENCE_SERIALIZE (1 << 1)

// EFER
#define X86_EFER_SCE 0x00000001             /* Enable SYSCALL */
#define X86_EFER_LME 0x00000100             /* Long mode enable */
#define X86_EFER_LMA 0x00000400             /* Long mode active */
#define X86_EFER_NXE 0x00000800             /* To enable execute disable bit */

#define X86_MSR_AMD_LS_CFG 0xc0011020 // Load/store unit configuration
#define X86_AMD_LS_CFG_F15H_SSBD (1ull << 54)
#define X86_AMD_LS_CFG_F16H_SSBD (1ull << 33)
#define X86_AMD_LS_CFG_F17H_SSBD (1ull << 10)
#define X86_MSR_K7_HWCR 0xc0010015               // AMD Hardware Configuration
#define X86_MSR_K7_HWCR_CPB_DISABLE (1ull << 25) // Set to disable turbo ('boost')

// KVM MSRs
#define X86_MSR_KVM_PV_EOI_EN 0x4b564d04 // Enable paravirtual fast APIC EOI
#define X86_MSR_KVM_PV_EOI_EN_ENABLE (1ull << 0)

#endif // #ifndef ARCH_MACRO_MSR_HPP