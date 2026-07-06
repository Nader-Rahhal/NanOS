#pragma once
#include <stdint.h>
#include "util.h"
#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "drivers/serial.h"
#include "memory/allocator.h"

#define TITLE_COLOR       Color::BLACK
#define BORDER_COLOR      Color::SILVER
#define TEXT_CURSOR_COLOR Color::WHITE
#define TEXT_COLOR        Color::WHITE
#define MAX_BUFFER_SIZE 64

#define CURSOR_WIDTH  12
#define CURSOR_HEIGHT 19

// 0 = transparent (leave background untouched), 1 = black border, 2 = white fill
static const uint8_t retro_cursor_bitmap[CURSOR_HEIGHT][CURSOR_WIDTH] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,2,2,2,2,1,0},
    {1,2,2,2,2,2,1,1,1,1,1,1},
    {1,2,2,1,2,2,1,0,0,0,0,0},
    {1,2,1,0,1,2,2,1,0,0,0,0},
    {1,1,0,0,1,2,2,1,0,0,0,0},
    {1,0,0,0,0,1,2,2,1,0,0,0},
    {0,0,0,0,0,1,2,2,1,0,0,0},
    {0,0,0,0,0,0,1,1,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0},
};

class WindowManager;

class Window {
private:
    friend class WindowManager;

    Window() : fb(nullptr), title(nullptr), bg(Color::BLACK), dims({0,0}), origin({0,0}), cursor(0, 0) {}

    Window(FrameBuffer* fb, Point origin, Dimensions dims, Color bg, const char* name, uint32_t cursorRow, uint32_t cursorCol, bool active = false)
    : fb(fb), title(name), bg(bg), dims(dims), origin(origin), cursor(cursorRow, cursorCol), active(active) {}

public:
    void draw() {
        fb->draw_rectangle(origin, dims, bg);
        draw_borders();
        draw_title();
        draw_corner_dot();
        if (active) {
            draw_text_cursor();
        }
    }

    Point     inner_origin() const { return { origin.x + side_border, origin.y + top_border }; }
    uint32_t  inner_width()  const { return dims.width  - 2 * side_border; }
    uint32_t  inner_height() const { return dims.height - top_border - side_border; }


    void resolve_scancode(uint8_t scancode) {
        if (scancode & 0x80) return;

        uint32_t max_cols = inner_width()  / 8;
        uint32_t max_rows = inner_height() / 16;

        erase_text_cursor();

        switch (scancode) {
            case 0x1C:
                cursor.newline(max_rows);
                wipe_buffer();
                break;
            case 0x0E:
                if (cursor.get_col() > 0) {
                    cursor.move_left();
                    Point inner = inner_origin();
                    fb->draw_rectangle({inner.x + cursor.get_col() * 8, inner.y + cursor.get_row() * 16}, {8, 16}, bg);
                    if (idx > 0) { idx--; buf[idx] = 0; }
                }
                break;
            default:
                char ch = scancode_to_char(scancode);
                if (ch == 0) break;
                buf[idx] = ch;
                idx++;
                char str[2] = {ch, '\0'};
                Point inner = inner_origin();
                fb->draw_string(str, {inner.x + cursor.get_col() * 8, inner.y + cursor.get_row() * 16}, TEXT_COLOR);
                cursor.move_right(max_cols);
                break;
        }

        draw_text_cursor();
    }

    void erase(Color bg) {
        fb->draw_rectangle(origin, dims, bg);
    }

    struct Dimensions get_dims(){
        return dims;
    }

    struct Point get_origin(){
        return origin;
    }

private:
    void draw_borders() {
        uint32_t x = origin.x, y = origin.y;
        uint32_t w = dims.width, h = dims.height;
        uint32_t tb = top_border, sb = side_border;

        fb->draw_rectangle({x,          y         }, {w,  tb}, BORDER_COLOR);
        fb->draw_rectangle({x,          y + h - sb}, {w,  sb}, BORDER_COLOR);
        fb->draw_rectangle({x,          y         }, {sb, h }, BORDER_COLOR);
        fb->draw_rectangle({x + w - sb, y         }, {sb, h }, BORDER_COLOR);
    }

    void draw_title() {
        uint32_t title_px = util::strlen(title) * 8;
        uint32_t px = origin.x + (dims.width - title_px) / 2;
        uint32_t py = origin.y + (top_border - 16) / 2;
        fb->draw_string(title, {px, py}, TITLE_COLOR);
    }

    void draw_corner_dot() {
        uint32_t radius = 6;
        Point center = { origin.x + dims.width - side_border - radius - 4, origin.y + top_border / 2 };
        fb->draw_circle(center, radius, Color::RED);
    }

    void draw_text_cursor() {
        Point inner = inner_origin();
        uint32_t px = inner.x + cursor.get_col() * 8;
        uint32_t py = inner.y + cursor.get_row() * 16;
        fb->draw_rectangle({px, py + 14}, {8, 2}, TEXT_CURSOR_COLOR);
    }

    void erase_text_cursor() {
        Point inner = inner_origin();
        uint32_t px = inner.x + cursor.get_col() * 8;
        uint32_t py = inner.y + cursor.get_row() * 16;
        fb->draw_rectangle({px, py + 14}, {8, 2}, bg);
    }

    void wipe_buffer() {
        for (uint32_t i = 0; i < MAX_BUFFER_SIZE; i++)
            buf[i] = 0;
        idx = 0;
    }

    bool active;
    FrameBuffer*  fb;
    const char*   title;
    Color         bg;
    uint32_t      top_border  = 20;
    uint32_t      side_border = 5;
    Dimensions    dims;
    Point         origin;
    TextCursor    cursor;
    char buf[MAX_BUFFER_SIZE];
    uint32_t idx = 0;
};

inline void* operator new(size_t, void* ptr) noexcept { return ptr; }

#define MAXIMUM_NUMBER_WINDOWS 4

class WindowManager {
public:

    WindowManager(){}

    WindowManager(FrameBuffer* fb, Color desktop_bg, PhysicalMemoryAllocator* allocator)
    : mouse(fb->get_width(), fb->get_height()), fb(fb), numWindows(0),
    desktop_bg(desktop_bg), allocator(allocator) {
        mouse.init();
        children = (Window**)allocator->malloc(sizeof(Window*) * MAXIMUM_NUMBER_WINDOWS);
    }

    uint32_t create_window(Point origin, Dimensions dims, Color bg, const char* name, uint32_t cursorRow = 0, uint32_t cursorCol = 0) {
        if (numWindows >= MAXIMUM_NUMBER_WINDOWS) return -1;
        bool active = (numWindows == 0); // first window is active by default

        void* mem = allocator->malloc(sizeof(Window));
        if (!mem) return -1;

        Window* win = new (mem) Window(fb, origin, dims, bg, name, cursorRow, cursorCol, active);
        children[numWindows] = win;
        return numWindows++;
    }

    void kill_window(uint32_t id) {

    }

    void draw() {
        
        for (uint32_t i = 0; i < numWindows; i++)
            children[i]->draw();
    }

    void change_active(uint8_t next_active){
        // set current active window to inactive
        children[active_window_id]->active = false;
        children[active_window_id]->erase_text_cursor();
        // set next active window to active
        children[next_active]->active = true;
        children[next_active]->draw_text_cursor();
        active_window_id = next_active;
    }

    bool read_packet(){
        return mouse.read_packet();
    }

    void redraw_mouse(){

        bool left_clicked = mouse.left_clicked();
        struct Point point = mouse.get_old_position();

        if (left_clicked) {
            for (uint32_t idx = 0; idx < numWindows; idx++) {
                Window* win = children[idx];
                Dimensions dims = win->get_dims();
                Point origin = win->get_origin();

                if (point.x >= origin.x && point.x <= origin.x + dims.width &&
                    point.y >= origin.y && point.y <= origin.y + dims.height) {
                    change_active(idx);
                    break;
                }
            }
        }

        int32_t prev_cx = (int32_t)point.x;
        int32_t prev_cy = (int32_t)point.y;

        if (!first_draw) {
            for (int32_t row = 0; row < CURSOR_HEIGHT; row++) {
                for (int32_t col = 0; col < CURSOR_WIDTH; col++) {
                    if (retro_cursor_bitmap[row][col] == 0) continue;
                    int32_t x = prev_cx + col;
                    int32_t y = prev_cy + row;
                    if (x < 0 || y < 0) continue;
                    if ((uint32_t)x >= fb->get_width() || (uint32_t)y >= fb->get_height()) continue;
                    RGB px = pixel_under_mouse.pixels[row][col];
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, px.r, px.g, px.b);
                }
            }
        }

        point = mouse.get_position();
        int32_t cx = (int32_t)point.x;
        int32_t cy = (int32_t)point.y;

        for (int32_t row = 0; row < CURSOR_HEIGHT; row++) {
            for (int32_t col = 0; col < CURSOR_WIDTH; col++) {
                uint8_t kind = retro_cursor_bitmap[row][col];
                if (kind == 0) continue;
                int32_t x = cx + col;
                int32_t y = cy + row;
                if (x < 0 || y < 0) continue;
                if ((uint32_t)x >= fb->get_width() || (uint32_t)y >= fb->get_height()) continue;

                pixel_under_mouse.pixels[row][col] = fb->get_pixel({(uint32_t)x, (uint32_t)y});

                if (kind == 1)
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, 0x00, 0x00, 0x00);
                else
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, 0xFF, 0xFF, 0xFF);
            }
        }

        first_draw = false;
    }

        

    void process_scancode(uint8_t code) {
        if (numWindows > 0)
            children[active_window_id]->resolve_scancode(code);
    }

private:

    struct PixelUnderMouse {
        RGB pixels[CURSOR_HEIGHT][CURSOR_WIDTH];
    } pixel_under_mouse;

    Mouse mouse;
    FrameBuffer* fb;
    PhysicalMemoryAllocator* allocator;

    uint32_t     numWindows;
    Window**   children; // array of window pointers
    Color        desktop_bg;
    uint8_t      active_window_id = 0;
    bool first_draw = true;
};

