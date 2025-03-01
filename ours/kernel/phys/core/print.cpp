#include <ours/phys/print.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/fmt/format_to.hpp>
#include <ustl/collections/intrusive/list.hpp>

namespace ours::phys {
    static ustl::collections::intrusive::List<Console, Console::ManagedOptions>  CONSOLE_LIST{};

    auto Console::init() -> void
    {
        ustl::mem::construct_at(&CONSOLE_LIST);
    }

    auto Console::activate() -> void
    {  CONSOLE_LIST.push_back(*this);  }

    auto Console::deactivate() -> void
    {  
        auto const to_erase = CONSOLE_LIST.iterator_to(*this);
        CONSOLE_LIST.erase(to_erase);
    }

    static char FORMAT_BUFFER[512];
    auto do_print(ustl::views::StringView fmt, const ustl::fmt::FormatArgs &args) -> void
    {
        std::string x;
        auto end = ustl::fmt::vformat_to(FORMAT_BUFFER, fmt, args);
        for (auto &console : CONSOLE_LIST) {
            console.write(FORMAT_BUFFER, end - FORMAT_BUFFER);
        }
    }

} // namespace ours::phys

namespace ours {
    auto panic() -> void
    {
        phys::do_print("panic", {});
        while (1);
    }

    NO_RETURN
    auto do_panic(ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void
    {
        phys::do_print("panic", args);
        while (1);
    }
}