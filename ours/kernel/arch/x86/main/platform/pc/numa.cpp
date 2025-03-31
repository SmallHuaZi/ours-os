namespace ours::phsy {
    static bool sNumaEnabled = true;

    auto arch_init_numa() -> void {
        if (!sNumaEnabled) {
            return;
        }

#ifdef OURS_CONFIG_ACPI_NUMA
        // if (Status::Ok != acpi_init_numa(global_handoff()->acpi_rsdp)) {
        //     return;
        // }
#endif
    }

} // namespace ours::phsy 