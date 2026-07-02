#pragma once
#include <stdint.h>

#define SET_SCALING_1_TO_1 0xE6
#define SET_SCALING_2_TO_1 0xE7
#define SET_RESOLUTION 0xE8
#define STATUS_REQUEST 0xE9
#define SET_STREAM_MODE 0xEA
#define READ_DATA 0xEB
#define RESET_WRAP_MODE 0xEC
#define SET_WRAP_MODE 0xEE
#define SET_REMOTE_MODE 0xF0
#define GET_DEVICE_ID 0xF2
#define SET_SAMPLE_RATE 0xF3
#define ENABLE_DATA_REPORTING 0xF4
#define DISABLE_DATA_REPORTING 0xF5
#define SET_DEFAULTS 0xF6
#define RESEND 0xFE
#define RESET 0xFF
#define MOUSE_ACK 0xFA
#define MOUSE_DEVICE_ID 0x00
#define DATA_PORT 0x60
#define CMD_PORT 0x64

struct __attribute__((packed)) MousePacket {
    uint8_t flags;
    int8_t x_movement;
    int8_t y_movement;
};

struct MouseState {
    bool overflow_y;
    bool overflow_x;
    bool negative_y;
    bool negative_x;
    bool middle_button_pressed;
    bool right_button_pressed;
    bool left_button_pressed;
    int32_t x;
    int32_t y;
};


class Mouse {
public:
    Mouse(uint32_t width, uint32_t height) : state{false, false, false, false, false, false, false, 0, 0} {
        fb_width = width;
        fb_height = height;
    }

    void init(){

        mouse_write(RESET);
        mouse_read(); // ACK
        mouse_read(); // 0xAA
        mouse_read(); // 0x00

        // set sample rate
        mouse_write(SET_SAMPLE_RATE);
        mouse_read();
        mouse_write(100);
        mouse_read();

        // set resolution
        mouse_write(SET_RESOLUTION);
        mouse_read();
        mouse_write(4);
        mouse_read();

        // set scaling 1:1
        mouse_write(SET_SCALING_1_TO_1);
        mouse_read();
        
        // start recording packets
        mouse_write(ENABLE_DATA_REPORTING);
        mouse_read();
    }

    void read_packet(){
        struct MousePacket packet;
        packet.flags = mouse_read();
        packet.x_movement = mouse_read();
        packet.y_movement = mouse_read();

        state.left_button_pressed   = packet.flags & (1 << 0);
        state.right_button_pressed  = packet.flags & (1 << 1);
        state.middle_button_pressed = packet.flags & (1 << 2);
        state.negative_x            = packet.flags & (1 << 4);
        state.negative_y            = packet.flags & (1 << 5);
        state.overflow_x            = packet.flags & (1 << 6);
        state.overflow_y            = packet.flags & (1 << 7);

        int32_t x_delta = (uint8_t)packet.x_movement - (state.negative_x ? 256 : 0);
        int32_t y_delta = (uint8_t)packet.y_movement - (state.negative_y ? 256 : 0);

        state.x += x_delta;
        state.y += y_delta;

        if (state.x < 0) state.x = 0;
        if (state.x >= (int32_t)fb_width) state.x = fb_width - 1;

        if (state.y < 0) state.y = 0;
        if (state.y >= (int32_t)fb_height) state.y = fb_height - 1;
    }

private:

    uint32_t fb_width;
    uint32_t fb_height;
    MouseState state;


    void __outb(uint16_t port, uint8_t val) {
        __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
    }

    uint8_t __inb(uint16_t port) {
        uint8_t val;
        __asm__ volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
        return val;
    }

    void wait_input() {
        while (__inb(CMD_PORT) & 0x2);
    }

    void wait_output() {
        while (!(__inb(CMD_PORT) & 0x1));
    }

    void mouse_write(uint8_t cmd) {
        wait_input();
        __outb(CMD_PORT, 0xD4);
        wait_input();
        __outb(DATA_PORT, cmd);
    }

    uint8_t mouse_read() {
        wait_output();
        return __inb(DATA_PORT);
    }

};