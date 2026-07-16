#pragma once
#include <stdint.h>

#include "util.h"
#include "color.h"
#include "drivers/keyboard.h"
#include "drivers/mouse.h"
#include "text_cursor.h"

class FrameBuffer {
    private:

    uint32_t get_color_hex(Color color) {
        return (uint32_t)color;
    }

    uint8_t* glyphs;
    uint32_t width;
    uint32_t height;
    uint64_t base;
    uint32_t pitch;    
    uint64_t size;
    uint64_t original_base;

    public:

    FrameBuffer(uint32_t w, uint32_t h, uint64_t b, uint32_t p, uint64_t s) : width(w), height(h), base(b), 
    pitch(p), size(s), original_base(base) {}

    void set_font(uint8_t address[]){
        glyphs = address + 4;
    }

    uint32_t get_width()  { return width;  }
    uint32_t get_height() { return height; }
    uint64_t get_size() { return size; }

    void draw_string(const char* str, Point p, Color color) {
        uint32_t base_x = p.x, base_y = p.y, col = 0;

        for (uint32_t i = 0; str[i] != '\0'; i++) {
            uint8_t* glyph = get_glyph((uint8_t)str[i]);

            for (uint32_t row = 0; row < 16; row++) {
                uint8_t bits = glyph[row];
                for (int bit = 0; bit < 8; bit++) {
                    if (bits & (0x80 >> bit))
                        draw_pixel({base_x + col * 8 + (uint32_t)bit, base_y + row}, color);
                }
            }

            col++;
            if (base_x + col * 8 >= width) {
                col = 0;
                base_y += 16;
            }
        }
    }

    void draw_image(RGB* image, Point p, Dimensions d) {
        for (uint32_t row = 0; row < d.height; row++) {
            for (uint32_t col = 0; col < d.width; col++) {
                const RGB& px = image[row * d.width + col];
                set_pixel({p.x + col, p.y + row}, px.r, px.g, px.b);
            }
        }
    }

    void draw_rectangle(Point p, Dimensions d, Color color) {
        for (uint32_t row = p.x; row < d.width + p.x; row++) {
            for (uint32_t col = p.y; col < d.height + p.y; col++) {
                draw_pixel({row, col}, color);
            }
        }
    }

    void draw_square(Point p, uint32_t len, Color color) {
        for (uint32_t row = p.x; row < len + p.x; row++) {
            for (uint32_t col = p.y; col < len + p.y; col++) {
                draw_pixel({row, col}, color);
            }
        }
    }

    void draw_circle(Point center, uint32_t radius, Color color) {
        int32_t r = (int32_t)radius;
        int32_t cx = (int32_t)center.x, cy = (int32_t)center.y;

        int32_t x = r;
        int32_t y = 0;
        int32_t err = 1 - r;

        while (x >= y) {
            draw_horizontal_span(cx - x, cx + x, cy + y, color);
            draw_horizontal_span(cx - x, cx + x, cy - y, color);
            draw_horizontal_span(cx - y, cx + y, cy + x, color);
            draw_horizontal_span(cx - y, cx + y, cy - x, color);

            y++;
            if (err < 0) {
                err += 2 * y + 1;
            } else {
                x--;
                err += 2 * (y - x) + 1;
            }
        }
    }

    void set_pixel(Point p, uint8_t r, uint8_t g, uint8_t b) {
        uint32_t* pixel = (uint32_t*)(base + p.y * pitch + p.x * 4);
        *pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    RGB get_pixel(Point p) {
        uint32_t* pixel = (uint32_t*)(base + p.y * pitch + p.x * 4);
        uint32_t v = *pixel;
        RGB out = { (uint8_t)((v >> 16) & 0xFF), (uint8_t)((v >> 8) & 0xFF), (uint8_t)(v & 0xFF) };
        return out;
    }

    void draw_line(Point p0, Point p1, Color color, uint32_t thickness = 1) {
        int32_t x0 = (int32_t)p0.x, y0 = (int32_t)p0.y;
        int32_t x1 = (int32_t)p1.x, y1 = (int32_t)p1.y;
        int32_t dx =  (x1 > x0 ? x1 - x0 : x0 - x1);
        int32_t dy = -(y1 > y0 ? y1 - y0 : y0 - y1);
        int32_t sx = x0 < x1 ? 1 : -1;
        int32_t sy = y0 < y1 ? 1 : -1;
        int32_t err = dx + dy;
        int32_t r = (int32_t)thickness / 2;

        while (true) {
            for (int32_t oy = -r; oy <= r; oy++)
                for (int32_t ox = -r; ox <= r; ox++)
                    draw_pixel({(uint32_t)(x0 + ox), (uint32_t)(y0 + oy)}, color);
            if (x0 == x1 && y0 == y1) break;
            int32_t e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void draw_pixel(Point p, Color color) {
        if (p.x >= width || p.y >= height)
            return;
        uint32_t* pixel = (uint32_t*)(base + p.y * pitch + p.x * 4);
        *pixel = get_color_hex(color);
    }

    void set_background(Color color) {
        for (uint32_t y = 0; y < height; y++)
            for (uint32_t x = 0; x < width; x++)
                draw_pixel({x, y}, color);
    }

    void set_buffer_to_back(uint64_t* buf){
        base = (uint64_t)buf;
    }

    void set_buffer_to_original(){
        base = original_base;
    }

    void copy_back_buffer(uint64_t* backbuf) {
        util::memcpy((void*)original_base, backbuf, size);
    }

    private:

    uint8_t* get_glyph(uint8_t c) {
        return glyphs + (c * 16);
    }

    void draw_horizontal_span(int32_t x0, int32_t x1, int32_t y, Color color) {
        if (y < 0) return;
        for (int32_t x = x0; x <= x1; x++) {
            if (x < 0) continue;
            draw_pixel({(uint32_t)x, (uint32_t)y}, color);
        }
    }
};
