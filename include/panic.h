#pragma once
#include "util.h"

#define KPANIC(msg)                          \
    do {                                     \
        util::serial_puts("\r\nKERNEL PANIC: ");   \
        util::serial_puts(msg);                    \
        util::serial_puts("\r\n");                 \
        for (;;) __asm__ volatile ("hlt");   \
    } while (0)
