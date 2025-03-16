#include <ours/phys/console.hpp>
#include <ours/phys/print.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/fmt/format_to.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::phys {
    [[gnu::init_priority(999)]]
    static ustl::collections::intrusive::List<Console, Console::ManagedOptions>  CONSOLE_LIST;

    Console::Console(char const *name)
        : hook_(),
          name_(name) {

        CONSOLE_LIST.push_back(*this);
    }

    auto init_early_console() -> void
    {
        for (auto &console : CONSOLE_LIST) {
            console.activate();
        }
    }

    auto vprint(ustl::views::StringView fmt) -> void
    {
        for (auto &console : CONSOLE_LIST) {
            console.write(fmt.begin(), fmt.size());
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