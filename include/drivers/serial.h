#pragma once
#include <stdint.h>

#define SERIAL_COM1 0x3F8

namespace drivers::serial::detail {

uint8_t inb(uint16_t port) {
    uint8_t val;
    __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

void print_char(char c) {
    while (!(inb(SERIAL_COM1 + 5) & 0x20));
    outb(SERIAL_COM1, c);
}

}

namespace drivers::serial {

void init() {
    detail::outb(SERIAL_COM1 + 1, 0x00);   // disable interrupts
    detail::outb(SERIAL_COM1 + 3, 0x80);   // DLAB on
    detail::outb(SERIAL_COM1 + 0, 0x01);   // divisor low: 115200 baud
    detail::outb(SERIAL_COM1 + 1, 0x00);   // divisor high
    detail::outb(SERIAL_COM1 + 3, 0x03);   // 8N1, DLAB off
    detail::outb(SERIAL_COM1 + 2, 0xC7);   // FIFO on, clear, 14-byte threshold
}

void print(const char* s) {
    while (*s) detail::print_char(*s++);
}

void print(uint64_t val) {
    if (val == 0) {
        detail::print_char('0');
        return;
    }
    char buf[20];
    int i = 0;
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i-- > 0) detail::print_char(buf[i]);
}

}