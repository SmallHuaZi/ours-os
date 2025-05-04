#include <ours/platform/hpet.hpp>
#include <ours/platform/acpi.hpp>
#include <ours/arch/apic.hpp>
#include <ours/assert.hpp>

// This is a very inconvenient way 
#include <ours/mem/vm_aspace.hpp>
#include <ours/mem/vm_area.hpp>
#include <ours/mem/vm_mapping.hpp>

#include <acpi/details/hpet.hpp>
#include <logz4/log.hpp>
#include <gktl/init_hook.hpp>
#include <ktl/new.hpp>
#include <ustl/chrono/duration.hpp>

using namespace ustl::chrono;

namespace ours {
    Hpet *g_hpet = 0;

    /// From https://wiki.osdev.org/HPET#Initialization, we should do the following:
    /// 1. Find HPET base address in 'HPET' ACPI table.
    /// 2. Calculate HPET frequency (f = 10^15 / period).
    /// 3. Save minimal tick (either from ACPI table or configuration register).
    /// 4. Initialize comparators.
    /// 5. Set ENABLE_CNF bit.
    static auto init_hpet() -> void {
        DEBUG_ASSERT(get_acpi_parser());
        auto hpet = get_table_by_type<acpi::AcpiHpet>(*get_acpi_parser());

        if (!hpet) {
            log::info("No HPET table found.");
            return;
        }

        if (hpet->address.address_space_id != acpi::kAspaceSysMemId) {
            log::info("HPET: MMIO unsupported.");
            return;
        }

        using namespace mem;
        VirtAddr hpet_mmio = 0;
        auto result = VmAspace::kernel_aspace()->root_vma()->map_at(
            hpet->address.address, 
            &hpet_mmio, 
            PAGE_SIZE,
            MmuFlags::Readable | MmuFlags::Writable | MmuFlags::Discache,
            VmMapOption::Commit | VmMapOption::Pinned,
            "HPET"
        );

        if (!result) {
            log::info("Failed to create HPET MMIO: {}.", to_string(result.unwrap_err()));
            return;
        }

        acpi::HpetRegs *hpet_regs = reinterpret_cast<acpi::HpetRegs *>(hpet_mmio);
        if (!hpet_regs->general_caps.support_64bit()) {
            log::info("HPET: Unsupport 64-bit.");
            return;
        }

        /// Usually three timers. But we need at least two timers, one used for the tick timer and another for
        /// TSC emulation.
        auto const num_timers = hpet_regs->general_caps.num_timers();
        if (num_timers < 2) {
            log::info("HPET: Not enough timers to support system requirements.");
            return;
        }

        // Make sure we disable all timers before we start configuring them.
        for (auto i = 0; i < num_timers; ++i) {
            hpet_regs->timers[i].conf_caps.mask();
        }

        CXX11_CONSTEXPR
        auto const kFemtosecondsPerSection = duration_cast<Femtoseconds>(Seconds(1)).count();
        auto const period = hpet_regs->general_caps.period(); 
        if (period == 0) {
            log::info("HPET: Invalid period.");
            return;
        }

        auto const nominal_frequency = (kFemtosecondsPerSection + period) / period;

        g_hpet = new (mem::kGafKernel) Hpet();
        DEBUG_ASSERT(g_hpet);
        g_hpet->mmio = hpet_regs;
        g_hpet->num_channels = num_timers;
        g_hpet->ticks_per_ms = nominal_frequency / 1000;
    }
    GKTL_INIT_HOOK(HpetInit, init_hpet, gktl::InitLevel::Vmm + 2);

    INIT_CODE
    auto enable_hpet() -> void {
        using namespace arch;
        /// TODO(SmallHuaZi) Remove this temporary workaround to HPET timer.
        g_hpet->disable();
        g_hpet->mmio->main_counter_value = 0;
        g_hpet->mmio->general_config.enable_legacy_route(true);

        // Periodic timer.
        g_hpet->mmio->timers[0].conf_caps.enable_periodic()
                                         .mask(false);
        // Set the comparator match
        g_hpet->mmio->timers[0].comparator_value = g_hpet->ticks_per_ms * 10;
        g_hpet->mmio->timers[0].comparator_value = g_hpet->ticks_per_ms * 10;

        g_hpet->enable();

        apic_configure_isa_irq(0, ApicDeliveryMode::Fixed, false, ApicDestinationMode::Physical, 
            0, IrqVec(u8(IrqVec::PlatformIrqMin) + 2));
    }

} // namespace ours