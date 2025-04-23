#include <logz4/log.hpp>

#include <ustl/fmt/format_to.hpp>

#include <arch/x86/io.hpp>

namespace logz4 {
    static auto write(char const *s, int n) -> void {
        auto const port_ = 0x3F8;
        auto const is_transmit_empty = [&]() { return arch::inb(port_ + 5) & 0x20; };

        for (auto i = 0; i < n; ++i) {
            while (!is_transmit_empty())
                ;
            arch::outb(port_, s[i]);
        }
    }

    auto do_log(Logger &logger, Level level, ustl::views::StringView fmt, ustl::fmt::FormatArgs const &args) -> void {
        // logger.log({ 
        //     .level = level,
        //     .args = args,
        //     .fmtstr = fmt,
        // });

        switch (level) {
        #define STR(S) S, sizeof(S)
            case Level::Status:
                write(STR("[status]: "));
                break;
            case Level::Warn:
                write(STR("[warn]: "));
                break;
            case Level::Info:
                write(STR("[info]: "));
                break;
            case Level::Debug:
                write(STR("[debug]: "));
                break;
            case Level::Trace:
                write(STR("[trace]: "));
                break;
            case Level::Error:
                write(STR("[error]: "));
                break;
        }

        static char s_format_buffer[512];

        auto end = ustl::fmt::vformat_to(s_format_buffer, fmt, args);
        *end = '\n';
        write(s_format_buffer, end - s_format_buffer + 1);
    }

    auto do_wlog(Logger &logger, Level level, ustl::views::WStringView fmt, ustl::fmt::FormatArgs const &args) -> void {
    }

} // namespace logz4