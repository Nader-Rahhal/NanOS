#pragma once
#include "util.h"

namespace arch::pic {

inline void disable() {
    util::outb(0x20, 0x11);  util::io_wait();
    util::outb(0xA0, 0x11);  util::io_wait();
    util::outb(0x21, 0x20);  util::io_wait();
    util::outb(0xA1, 0x28);  util::io_wait();
    util::outb(0x21, 0x04);  util::io_wait();
    util::outb(0xA1, 0x02);  util::io_wait();
    util::outb(0x21, 0x01);  util::io_wait();
    util::outb(0xA1, 0x01);  util::io_wait();
    util::outb(0x21, 0xFF);
    util::outb(0xA1, 0xFF);
}

}
