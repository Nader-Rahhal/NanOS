#pragma once
#include <stdint.h>
#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "memory/allocator.h"
#include "kstate.h"
#include "kapi.h"
#include "nstd/managed_array.h"

#define TEXT_CURSOR_COLOR Color::WHITE
#define TEXT_COLOR        Color::WHITE

#define MAX_BUFFER_SIZE 64

class Application {
public:

    virtual ~Application() {}

    virtual void handle_char(char ch) = 0;

    void redraw() {
        for (uint32_t row = 0; row < max_rows; row++) {
            for (uint32_t col = 0; col < max_cols; col++) {
                char ch = grid[row * max_cols + col];
                if (ch == 0) continue;
                char str[2] = {ch, '\0'};
                fb->draw_string(str, cell_origin(row, col), TEXT_COLOR);
            }
        }
    }

    void draw_cursor() {
        fb->draw_rectangle(cursor_cell_offset(0, 14), {8, 2}, TEXT_CURSOR_COLOR);
    }

    void erase_cursor() {
        fb->draw_rectangle(cursor_cell_offset(0, 14), {8, 2}, bg);
    }

    void set_inner(Point inner_origin) { inner = inner_origin; }

protected:

    uint32_t grid_size() const { return max_rows * max_cols; }

    char& cell_at_cursor() {
        return grid[cursor.get_row() * max_cols + cursor.get_col()];
    }

    Point cell_origin(uint32_t row, uint32_t col) {
        return { inner.x + col * 8, inner.y + row * 16 };
    }

    Point cursor_cell_offset(uint32_t dx, uint32_t dy) {
        return { inner.x + cursor.get_col() * 8 + dx,
                 inner.y + cursor.get_row() * 16 + dy };
    }

    void echo_char(char ch) {
        char str[2] = {ch, '\0'};
        fb->draw_string(str, cursor_cell_offset(0, 0), TEXT_COLOR);
    }

    void erase_cell() {
        fb->draw_rectangle(cursor_cell_offset(0, 0), {8, 16}, bg);
    }

    void wipe_grid() {
        for (uint32_t i = 0; i < grid_size(); i++)
            grid[i] = 0;
    }

    void wipe_buffer() {
        buf.clear();
        idx = 0;
    }

protected:

    FrameBuffer*             fb;
    char*                    grid;
    Color                    bg;
    Point                    inner;
    uint32_t                 max_cols;
    uint32_t                 max_rows;
    TextCursor               cursor;
    nstd::managed_array<char, MAX_BUFFER_SIZE> buf;
    uint32_t                 idx;
    kernel_state*            kstate;
    kernel_api*              kapi;
};