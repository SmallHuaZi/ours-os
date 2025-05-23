#include <ours/platform/isa.hpp>
#include <ours/arch/apic.hpp>
#include <ours/irq/mod.hpp>

#include <arch/x86/io.hpp>
#include <logz4/log.hpp>
#include <ustl/io/circular_buffer.hpp>
#include <ktl/allocator.hpp>
#include <gktl/init_hook.hpp>

/// i8042 keyboard controller registers
#define I8042_COMMAND_REG 0x64
#define I8042_STATUS_REG 0x64
#define I8042_DATA_REG 0x60

/// timeout in milliseconds
#define I8042_CTL_TIMEOUT 500

/// status register bits
#define I8042_STR_PARITY 0x80
#define I8042_STR_TIMEOUT 0x40
#define I8042_STR_AUXDATA 0x20
#define I8042_STR_KEYLOCK 0x10
#define I8042_STR_CMDDAT 0x08
#define I8042_STR_MUXERR 0x04
#define I8042_STR_IBF 0x02
#define I8042_STR_OBF 0x01

/// control register bits
#define I8042_CTR_KBDINT 0x01
#define I8042_CTR_AUXINT 0x02
#define I8042_CTR_IGNKEYLK 0x08
#define I8042_CTR_KBDDIS 0x10
#define I8042_CTR_AUXDIS 0x20
#define I8042_CTR_XLATE 0x40

/// commands
#define I8042_CMD_CTL_RCTR 0x0120
#define I8042_CMD_CTL_WCTR 0x1060
#define I8042_CMD_CTL_TEST 0x01aa

#define I8042_CMD_KBD_DIS 0x00ad
#define I8042_CMD_KBD_EN 0x00ae
#define I8042_CMD_PULSE_RESET 0x00fe
#define I8042_CMD_KBD_TEST 0x01ab
#define I8042_CMD_KBD_MODE 0x01f0

/// Used for flushing buffers. the i8042 internal buffer shouldn't exceed this.
#define I8042_BUFFER_LENGTH 32

namespace ours {
    static inline auto i8042_read_data(void) -> u8 { 
        return arch::inb(I8042_DATA_REG); 
    }

    static inline auto i8042_read_status(void) -> u8 { 
        return arch::inb(I8042_STATUS_REG); 
    }

    static inline auto i8042_write_data(u8 val) -> void { 
        arch::outb(I8042_DATA_REG, val); 
    }

    static inline auto i8042_write_command(u8 val) -> void { 
        arch::outb(I8042_COMMAND_REG, val); 
    }

    /// extended keys that aren't pure ascii
    enum ExtendedKeys {
        KEY_RETURN = 0x80,
        KEY_ESC,
        KEY_LSHIFT,
        KEY_RSHIFT,
        KEY_LCTRL,
        KEY_RCTRL,
        KEY_LALT,
        KEY_RALT,
        KEY_CAPSLOCK,
        KEY_LWIN,
        KEY_RWIN,
        KEY_MENU,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_F11,
        KEY_F12,
        KEY_F13,
        KEY_F14,
        KEY_F15,
        KEY_F16,
        KEY_F17,
        KEY_F18,
        KEY_F19,
        KEY_F20,
        KEY_PRTSCRN,
        KEY_SCRLOCK,
        KEY_PAUSE,
        KEY_TAB,
        KEY_BACKSPACE,
        KEY_INS,
        KEY_DEL,
        KEY_HOME,
        KEY_END,
        KEY_PGUP,
        KEY_PGDN,
        KEY_ARROW_UP,
        KEY_ARROW_DOWN,
        KEY_ARROW_LEFT,
        KEY_ARROW_RIGHT,
        KEY_PAD_NUMLOCK,
        KEY_PAD_DIVIDE,
        KEY_PAD_MULTIPLY,
        KEY_PAD_MINUS,
        KEY_PAD_PLUS,
        KEY_PAD_ENTER,
        KEY_PAD_PERIOD,
        KEY_PAD_0,
        KEY_PAD_1,
        KEY_PAD_2,
        KEY_PAD_3,
        KEY_PAD_4,
        KEY_PAD_5,
        KEY_PAD_6,
        KEY_PAD_7,
        KEY_PAD_8,
        KEY_PAD_9,

        _KEY_LAST,
    };

    static_assert(_KEY_LAST < 0x100, "");

    /* scancode translation tables */
    const uint8_t kPcKeyMapSet1Lower[128] = {
        /* 0x00 */ 0,
        KEY_ESC,
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '0',
        '-',
        '=',
        KEY_BACKSPACE,
        KEY_TAB,
        /* 0x10 */ 'q',
        'w',
        'e',
        'r',
        't',
        'y',
        'u',
        'i',
        'o',
        'p',
        '[',
        ']',
        KEY_RETURN,
        KEY_LCTRL,
        'a',
        's',
        /* 0x20 */ 'd',
        'f',
        'g',
        'h',
        'j',
        'k',
        'l',
        ';',
        '\'',
        '`',
        KEY_LSHIFT,
        '\\',
        'z',
        'x',
        'c',
        'v',
        /* 0x30 */ 'b',
        'n',
        'm',
        ',',
        '.',
        '/',
        KEY_RSHIFT,
        '*',
        KEY_LALT,
        ' ',
        KEY_CAPSLOCK,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        /* 0x40 */ KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_PAD_NUMLOCK,
        KEY_SCRLOCK,
        KEY_PAD_7,
        KEY_PAD_8,
        KEY_PAD_9,
        KEY_PAD_MINUS,
        KEY_PAD_4,
        KEY_PAD_5,
        KEY_PAD_6,
        KEY_PAD_PLUS,
        KEY_PAD_1,
        /* 0x50 */ KEY_PAD_2,
        KEY_PAD_3,
        KEY_PAD_0,
        KEY_PAD_PERIOD,
        0,
        0,
        0,
        KEY_F11,
        KEY_F12,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    const uint8_t kPcKeyMapSet1Upper[128] = {
        /* 0x00 */ 0,
        KEY_ESC,
        '!',
        '@',
        '#',
        '$',
        '%',
        '^',
        '&',
        '*',
        '(',
        ')',
        '_',
        '+',
        KEY_BACKSPACE,
        KEY_TAB,
        /* 0x10 */ 'Q',
        'W',
        'E',
        'R',
        'T',
        'Y',
        'U',
        'I',
        'O',
        'P',
        '{',
        '}',
        KEY_RETURN,
        KEY_LCTRL,
        'A',
        'S',
        /* 0x20 */ 'D',
        'F',
        'G',
        'H',
        'J',
        'K',
        'L',
        ':',
        '"',
        '~',
        KEY_LSHIFT,
        '|',
        'Z',
        'X',
        'C',
        'V',
        /* 0x30 */ 'B',
        'N',
        'M',
        '<',
        '>',
        '?',
        KEY_RSHIFT,
        '*',
        KEY_LALT,
        ' ',
        KEY_CAPSLOCK,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        /* 0x40 */ KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,
        KEY_F10,
        KEY_PAD_NUMLOCK,
        KEY_SCRLOCK,
        KEY_PAD_7,
        KEY_PAD_8,
        KEY_PAD_9,
        KEY_PAD_MINUS,
        KEY_PAD_4,
        KEY_PAD_5,
        KEY_PAD_6,
        KEY_PAD_PLUS,
        KEY_PAD_1,
        /* 0x50 */ KEY_PAD_2,
        KEY_PAD_3,
        KEY_PAD_0,
        KEY_PAD_PERIOD,
        0,
        0,
        0,
        KEY_F11,
        KEY_F12,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };

    const uint8_t kPcKeyMapSet1E0[128] = {
        /* 0x00 */ 0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        /* 0x10 */ 0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        KEY_PAD_ENTER,
        KEY_RCTRL,
        0,
        0,
        /* 0x20 */ 0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        /* 0x30 */ 0,
        0,
        0,
        0,
        0,
        KEY_PAD_DIVIDE,
        0,
        KEY_PRTSCRN,
        KEY_RALT,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        /* 0x40 */ 0,
        0,
        0,
        0,
        0,
        0,
        0,
        KEY_HOME,
        KEY_ARROW_UP,
        KEY_PGUP,
        0,
        KEY_ARROW_LEFT,
        0,
        KEY_ARROW_RIGHT,
        0,
        KEY_END,
        /* 0x50 */ KEY_ARROW_DOWN,
        KEY_PGDN,
        KEY_INS,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        KEY_LWIN,
        KEY_RWIN,
        KEY_MENU,
        0,
        0};

    /// State key flags
    static bool key_lshift;
    static bool key_rshift;
    static int last_code;

    static ustl::io::CircularBuffer<u8, ktl::Allocator<u8>> key_buf;

    FORCE_INLINE
    static auto isprint(u8 c) -> bool {
        return 0x20 <= c && c <=0x7E;
    }

    static auto i8042_process_scode(u8 scode, i32 flags) -> void {
        // is this a multi code sequence?
        bool multi = (last_code == 0xe0);

        // update the last received code
        last_code = scode;

        // save the key up event bit
        bool key_up = !!(scode & 0x80);
        scode &= 0x7f;

        // translate the key based on our translation table
        uint8_t key_code;
        if (multi) {
            key_code = kPcKeyMapSet1E0[scode];
        } else if (key_lshift || key_rshift) {
            key_code = kPcKeyMapSet1Upper[scode];
        } else {
            key_code = kPcKeyMapSet1Lower[scode];
        }

        log::trace("scancode 0x%x, keyup %d, multi %d: keycode 0x%x\n", scode, !!key_up, multi, key_code);

        // generate a character string to feed into the queue
        char str[4] = {0};
        switch (key_code) {
            // for all the usual ascii strings, generate the target string directly
            case 1 ... 0x7f:
                str[0] = key_code;
                break;

            // a few special keys we can generate stuff for directly
            case KEY_RETURN:
            case KEY_PAD_ENTER:
                str[0] = '\n';
                break;
            case KEY_BACKSPACE:
                str[0] = '\b';
                break;
            case KEY_TAB:
                str[0] = '\t';
                break;

            // generate vt100 key codes for arrows
            case KEY_ARROW_UP:
                str[0] = 0x1b;
                str[1] = '[';
                str[2] = 65;
                break;
            case KEY_ARROW_DOWN:
                str[0] = 0x1b;
                str[1] = '[';
                str[2] = 66;
                break;
            case KEY_ARROW_RIGHT:
                str[0] = 0x1b;
                str[1] = '[';
                str[2] = 67;
                break;
            case KEY_ARROW_LEFT:
                str[0] = 0x1b;
                str[1] = '[';
                str[2] = 68;
                break;

            // left and right shift are special
            case KEY_LSHIFT:
                key_lshift = !key_up;
                break;
            case KEY_RSHIFT:
                key_rshift = !key_up;
                break;

            // everything else we just eat
            default:;  // nothing
        }

        if (!key_up) {
            for (u32 i = 0; str[i] != '\0'; ++i) {
                log::trace("char 0x{:X} ({})\n", str[i], isprint(str[i]) ? (str[i]) : ' ');
                key_buf.push_back(str[i]);
            }
        }
    }

    static auto spin(int const n) -> void {
        for (auto i = 0; i < n * 10; ++i) {
        }
    }

    static auto i8042_wait_read(void) -> i32 {
        int i = 0;
        while ((~i8042_read_status() & I8042_STR_OBF) && (i < I8042_CTL_TIMEOUT)) {
            spin(10);
            i++;
        }
        return -(i == I8042_CTL_TIMEOUT);
    }

    static auto i8042_wait_write(void) -> i32 {
        int i = 0;
        while ((i8042_read_status() & I8042_STR_IBF) && (i < I8042_CTL_TIMEOUT)) {
            spin(10);
            i++;
        }
        return -(i == I8042_CTL_TIMEOUT);
    }

    static auto i8042_flush(void) -> i32 {
        unsigned char data [[maybe_unused]];
        int i = 0;

        while ((i8042_read_status() & I8042_STR_OBF) && (i++ < I8042_BUFFER_LENGTH)) {
            spin(10);
            data = i8042_read_data();
        }

        return i;
    }

    static auto i8042_command(uint8_t* param, uint16_t command) ->i32 {
        int retval = 0, i = 0;

        retval = i8042_wait_write();
        if (!retval) {
            i8042_write_command(static_cast<uint8_t>(command));
        }

        if (!retval) {
            for (i = 0; i < ((command >> 12) & 0xf); i++) {
                if ((retval = i8042_wait_write())) {
                    break;
                }

                i8042_write_data(param[i]);
            }
        }

        if (!retval) {
            for (i = 0; i < ((command >> 8) & 0xf); i++) {
                if ((retval = i8042_wait_read())) {
                    break;
                }

                if (i8042_read_status() & I8042_STR_AUXDATA) {
                    param[i] = static_cast<uint8_t>(~i8042_read_data());
                } else {
                    param[i] = i8042_read_data();
                }
            }
        }

        return retval;
    }

    static auto keyboard_command(uint8_t* param, int command) -> i32 {
        int retval = 0, i = 0;

        retval = i8042_wait_write();
        if (!retval) {
            i8042_write_data(static_cast<uint8_t>(command));
        }

        if (!retval) {
            for (i = 0; i < ((command >> 12) & 0xf); i++) {
                if ((retval = i8042_wait_write())) {
                    break;
                }

                i8042_write_data(param[i]);
            }
        }

        if (!retval) {
            for (i = 0; i < ((command >> 8) & 0xf); i++) {
                if ((retval = i8042_wait_read())) {
                    break;
                }

                if (i8042_read_status() & I8042_STR_AUXDATA) {
                    param[i] = static_cast<uint8_t>(~i8042_read_data());
                } else {
                    param[i] = i8042_read_data();
                }
            }
        }

        return retval;
    }

    static auto i8042_interrupt(VIrqNum, void *) -> irq::IrqReturn {
        // keep handling status on the keyboard controller until no bits are set we care about
        bool retry;
        do {
            retry = false;

            uint8_t str = i8042_read_status();

            // check for incoming data from the controller
            if (str & I8042_STR_OBF) {
                uint8_t data = i8042_read_data();
                i8042_process_scode(data, ((str & I8042_STR_PARITY) ? I8042_STR_PARITY : 0) |
                                              ((str & I8042_STR_TIMEOUT) ? I8042_STR_TIMEOUT : 0));

                retry = true;
            }

            // TODO: check other status bits here
        } while (retry);

        return irq::IrqReturn::Handled;
    }

    static auto platform_init_keyboard() -> void {
        uint8_t ctr;

        key_buf.resize(KB(1));

        i8042_flush();

        if (i8042_command(&ctr, I8042_CMD_CTL_RCTR)) {
            log::debug("Failed to read CTR while initializing i8042\n");
            return;
        }

        // Turn on translation
        ctr |= I8042_CTR_XLATE;

        // Enable keyboard and keyboard IRQ
        ctr &= static_cast<u8>(~I8042_CTR_KBDDIS);
        ctr |= I8042_CTR_KBDINT;

        if (i8042_command(&ctr, I8042_CMD_CTL_WCTR)) {
            log::debug("Failed to write CTR while initializing i8042\n");
            return;
        }

        // Enable PS/2 port
        i8042_command(NULL, I8042_CMD_KBD_EN);

        // Send a enable scan command to the keyboard
        keyboard_command(&ctr, 0x1f4);

        uint32_t irq = apic_io_isa_to_global(ISA_IRQ_KEYBOARD);
        auto status = irq::request_irq(irq, i8042_interrupt, irq::IrqFlags::None, "Keyboard");
        apic_configure_isa_irq(ISA_IRQ_KEYBOARD, 
            arch::ApicDeliveryMode::Fixed, 
            false, 
            arch::ApicDestinationMode::Physical, 
            0, 
            arch::IrqVec(u8(arch::IrqVec::PlatformIrqMin) + irq)
        );
        DEBUG_ASSERT(status == Status::Ok);
        apic_unmask_irq(irq);

        i8042_interrupt(0, NULL);
    }
    GKTL_INIT_HOOK(PlatformInitKeyboard, platform_init_keyboard, gktl::InitLevel::Vmm + 3);

} // namespace ours