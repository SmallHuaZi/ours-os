#include <ours/phys/print.hpp>

#include <ours/types.hpp>
#include <ours/config.hpp>

#include <ustl/mem/object.hpp>
#include <ustl/fmt/format_to.hpp>

namespace ours::phys {
    enum class Color: u16 {
        Black = 0,
        Blue = 1,
        Green = 2,
        Cyan = 3,
        Red = 4,
        Magenta = 5,
        Brown = 6,
        LightGray = 7,
        DarkGray = 8,
        LightBlue = 9,
        LightGreen = 10,
        LightCyan = 11,
        LightRed = 12,
        Pink = 13,
        Yellow = 14,
        White = 15,
    };

    struct X86VgaConsole
        : public Console 
    {
        virtual auto write(char const *s, usize n) -> void override;

        virtual auto read(char const *s, usize n) -> usize override
        {  return 0; }

        auto print_char(Color back, Color fore, char c) -> void;

        auto clear_line(u16 line) -> void;

        char buffer_[512];
        Color foreground_ = Color::White;
        Color background_ = Color::Black;
        volatile u32 xpos_ = 0;
        volatile u32 ypos_ = 0;

        static volatile u8 *VIDEO_;
        CXX11_CONSTEXPR static auto const LINES = 25;
        CXX11_CONSTEXPR static auto const COLUMNS = 80;
    };
    volatile u8 *X86VgaConsole::VIDEO_ = reinterpret_cast<u8 *>(0xB8000);

    auto X86VgaConsole::write(char const *s, usize n) -> void
    {
        for (auto i = 0; i < n; ++i) {
            this->print_char(foreground_, background_, s[i]);
        }
    }

    auto X86VgaConsole::clear_line(u16 line) -> void
    {
        for (auto i = 0; i < COLUMNS; ++i) {
            auto const j = (COLUMNS * line + i) << 1;
            VIDEO_[j] = 0;
            VIDEO_[j + 1] = (u16(background_) << 4) | u16(foreground_);    
        } 
    }

    auto X86VgaConsole::print_char(Color back, Color fore, char c) -> void
    {
        if (c == '\n' || c == '\r' || xpos_ >= COLUMNS) {
            xpos_ = 0;
            ypos_ = (ypos_ + 1) % LINES;
            this->clear_line(ypos_);
            return;
        }

        auto const j = (COLUMNS * ypos_ + xpos_) << 1;
        VIDEO_[j] = c & 0xFF;
        VIDEO_[j + 1] = (u16(back) << 4) | u16(fore);
        xpos_ += 1;
    }

    static X86VgaConsole VGA_LOGGER;

    auto init_early_console() -> void
    {
        ustl::mem::construct_at(&VGA_LOGGER);
        for (int i = 0; i < 80; ++i) {
            VGA_LOGGER.clear_line(i);
        }

        VGA_LOGGER.activate();
    }

} // namespace ours::phys