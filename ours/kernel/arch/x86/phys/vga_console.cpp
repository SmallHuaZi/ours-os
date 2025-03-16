#include <ours/config.hpp>
#include <ours/phys/console.hpp>
#include <ours/types.hpp>

#include <ustl/fmt/format_to.hpp>
#include <ustl/mem/object.hpp>

namespace ours::phys {
    enum class Color : u16 {
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

    struct X86VgaConsole: public Console {
        X86VgaConsole(u8 *vga)
            : Console("VGA"),
              xpos_(0),
              ypos_(0),
              video_(vga),
              foreground_(Color::Black),
              background_(Color::White) {}

        auto write(char const *s, u32 n) -> void override;
        auto read(char *s, u32 n) -> u32 override {
            return 0;
        }

        auto print_char(Color back, Color fore, char c) -> void;
        auto clear_line(u16 line) -> void;

        auto activate() -> void override;

        Color foreground_;
        Color background_;
        volatile u32 xpos_;
        volatile u32 ypos_;
        volatile u8 *video_;

        CXX11_CONSTEXPR static auto const LINES = 25;
        CXX11_CONSTEXPR static auto const COLUMNS = 80;
    };

    auto X86VgaConsole::write(char const *s, u32 n) -> void {
        for (auto i = 0; i < n; ++i) {
            this->print_char(foreground_, background_, s[i]);
        }
    }

    auto X86VgaConsole::clear_line(u16 line) -> void {
        for (auto i = 0; i < COLUMNS; ++i) {
            auto const j = (COLUMNS * line + i) << 1;
            video_[j] = 0;
            video_[j + 1] = (u16(background_) << 4) | u16(foreground_);
        }
    }

    auto X86VgaConsole::print_char(Color back, Color fore, char c) -> void {
        if (c == '\n' || c == '\r' || xpos_ >= COLUMNS) {
            xpos_ = 0;
            ypos_ = (ypos_ + 1) % LINES;
            this->clear_line(ypos_);
            return;
        }

        auto const j = (COLUMNS * ypos_ + xpos_) << 1;
        video_[j] = c & 0xFF;
        video_[j + 1] = (u16(back) << 4) | u16(fore);
        xpos_ += 1;
    }

    auto X86VgaConsole::activate() -> void {
        for (int i = 0; i < COLUMNS; ++i) {
            clear_line(i);
        }
    }

    REGISTER_CONSOLE(X86VgaConsole, VGA_LOGGER, reinterpret_cast<u8 *>(0xB8000));

} // namespace ours::phys