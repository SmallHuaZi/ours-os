#include <ours/init.hpp>

namespace ours {
    // enum class Color: u16 {
    //     Black = 0,
    //     Blue = 1,
    //     Green = 2,
    //     Cyan = 3,
    //     Red = 4,
    //     Magenta = 5,
    //     Brown = 6,
    //     LightGray = 7,
    //     DarkGray = 8,
    //     LightBlue = 9,
    //     LightGreen = 10,
    //     LightCyan = 11,
    //     LightRed = 12,
    //     Pink = 13,
    //     Yellow = 14,
    //     White = 15,
    // };

    // class EarlyConsole
    //     : public Console
    // {
    // public:
    //     OURS_CONSOLE_API;

    //     virtual ~EarlyConsole()
    //     {}

    //     auto print(Color back, Color fore, char c) -> void;
    //     auto clear_line(usize const line) -> void;
    //     auto clear_screen() -> void;

    // private:
    //     volatile u32 xpos_ = 0;
    //     volatile u32 ypos_ = 0;
    //     volatile char *video_ = reinterpret_cast<char *>(0xB8000);

    //     Color default_foreground_ = Color::White;
    //     Color default_background_ = Color::Black;

    //     static constexpr auto const LINES = 25;
    //     static constexpr auto const COLUMNS = 80;
    // };

    // auto EarlyConsole::clear_line(usize const line) -> void
    // {
    //     for (auto i = 0; i < COLUMNS; ++i) {
    //         auto const j = (COLUMNS * line + i) << 1;
    //         video_[j] = 0;
    //         video_[j + 1] = (u16(default_background_) << 4) | u16(default_foreground_);
    //     }
    // }

    // INIT_CODE
    // auto EarlyConsole::print(Color back, Color fore, char c) -> void
    // {
    //     if (c == '\n' || c == '\r' || xpos_ >= COLUMNS) {
    //         xpos_ = 0;
    //         ypos_ = (ypos_ + 1) % LINES;
    //         this->clear_line(ypos_);
    //         return;
    //     }

    //     auto const j = (COLUMNS * ypos_ + xpos_) << 1;
    //     video_[j] = c & 0xFF;
    //     video_[j + 1] = (u16(back) << 4) | u16(fore);
    //     xpos_ += 1;
    // }

    // auto EarlyConsole::read() -> u32  
    // {
    //     return 0;
    // }

    // auto EarlyConsole::write(u32 c) -> void 
    // {
    //     print(default_background_, default_foreground_, c);
    // }

    // INIT_DATA 
    // static EarlyConsole S_EARLY_CONSOLE;

    // INIT_DATA 
    // Console *G_EARLY_CONSOLE = &S_EARLY_CONSOLE;

    // auto cls() -> void
    // {
    //     for (int i = 0; i < 80; ++i) {
    //         S_EARLY_CONSOLE.clear_line(i);
    //     }
    // }

} // namespace ours