#include <ours/phys/console.hpp>
#include <ours/phys/print.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/fmt/format_to.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::phys {
    static ustl::collections::intrusive::List<Console, Console::ManagedOptions>  CONSOLE_LIST;

    auto Console::activate() -> void
    {  CONSOLE_LIST.push_back(*this);  }

    auto Console::deactivate() -> void
    {  
        auto const to_erase = CONSOLE_LIST.iterator_to(*this);
        CONSOLE_LIST.erase(to_erase);
    }

    extern Console *PRESET_CONSOLE_START[] LINK_NAME("__preset_console_start");
    extern Console *PRESET_CONSOLE_END[] LINK_NAME("__preset_console_end");

    auto init_early_console() -> void
    {
        ustl::mem::construct_at(&CONSOLE_LIST);
        auto const n = PRESET_CONSOLE_END - PRESET_CONSOLE_START;
        for (auto i = 0; i < n; ++i) {
            PRESET_CONSOLE_START[i]->activate();
        }
    }

    auto vprint(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void
    {
        static char FORMAT_BUFFER[512];

        auto end = ustl::fmt::vformat_to(FORMAT_BUFFER, fmt, args);
        for (auto &console : CONSOLE_LIST) {
            console.write(FORMAT_BUFFER, end - FORMAT_BUFFER);
        }
    }

} // namespace ours::phys