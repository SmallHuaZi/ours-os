#include <ours/phys/console.hpp>
#include <ustl/mem/object.hpp>

#include <arch/x86/io.hpp>

namespace ours::phys {
    CXX11_CONSTEXPR
    static u16 const COM1 = 0x3F8;

    struct SerialConsole: public Console {
        SerialConsole()
            : Console("Serial"),
              port_(COM1) {
        }

        auto init() -> void;
        auto write(char const *s, u32 n) -> void override;
        auto read(char *s, u32 n) -> u32 override;
        auto activate() -> void override final;

        u16 const port_;
    };

    auto SerialConsole::write(char const *s, u32 n) -> void {
        auto const is_transmit_empty = [&]() { return arch::x86::inb(port_ + 5) & 0x20; };

        for (auto i = 0; i < n; ++i) {
            while (!is_transmit_empty())
                ;
            arch::x86::outb(port_, s[i]);
        }
    }

    auto SerialConsole::read(char *s, u32 n) -> u32 {
        auto const has_received = [&]() { return arch::x86::inb(port_ + 5) & 1; };

        for (auto i = 0; i < n; ++i) {
            while (!has_received())
                ;
            s[i] = arch::x86::inb(port_);
        }

        return n;
    }

    auto SerialConsole::activate() -> void {
        arch::x86::outb(port_ + 1, 0x00); // Disable all interrupts
        arch::x86::outb(port_ + 3, 0x80); // Enable DLAB (set baud rate divisor)
        arch::x86::outb(port_ + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
        arch::x86::outb(port_ + 1, 0x00); //                  (hi byte)
        arch::x86::outb(port_ + 3, 0x03); // 8 bits, no parity, one stop bit
        arch::x86::outb(port_ + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
        arch::x86::outb(port_ + 4, 0x0B); // IRQs enabled, RTS/DSR set
        arch::x86::outb(port_ + 4, 0x1E); // Set in loopback mode, test the serial chip
        arch::x86::outb(port_ + 0, 0xAE); // Test serial chip (send byte 0xAE and check if serial returns same byte)

        if (arch::x86::inb(port_ + 0) != 0xAE) {
            return;
        }

        // If serial is not faulty set it in normal operation mode
        // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
        arch::x86::outb(port_ + 4, 0x0F);
    }

    REGISTER_CONSOLE(SerialConsole, SERIAL_CONSOLE);

} // namespace ours::phys