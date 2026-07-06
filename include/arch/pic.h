#pragma once
#include "util.h"

inline void disable_pic() {
    util::outb(0x20, 0x11);  util::io_wait();  // ICW1: start init
    util::outb(0xA0, 0x11);  util::io_wait();
    util::outb(0x21, 0x20);  util::io_wait();  // ICW2: master base vector 0x20
    util::outb(0xA1, 0x28);  util::io_wait();  // ICW2: slave base vector 0x28
    util::outb(0x21, 0x04);  util::io_wait();  // ICW3: slave on IRQ2
    util::outb(0xA1, 0x02);  util::io_wait();  // ICW3: slave cascade identity
    util::outb(0x21, 0x01);  util::io_wait();  // ICW4: 8086 mode
    util::outb(0xA1, 0x01);  util::io_wait();
    util::outb(0x21, 0xFF);              // mask all master IRQs
    util::outb(0xA1, 0xFF);              // mask all slave IRQs
}
