#pragma once
#include <stdint.h>
#include <new>
#include "util.h"
#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "drivers/serial.h"
#include "memory/allocator.h"
#include "kstate.h"

#define TITLE_COLOR       Color::BLACK
#define BORDER_COLOR      Color::SILVER
#define TEXT_CURSOR_COLOR Color::WHITE
#define TEXT_COLOR        Color::WHITE
#define MAX_BUFFER_SIZE 64

#define CURSOR_WIDTH  12
#define CURSOR_HEIGHT 19

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


class ShellApplication {
public:
    ShellApplication()
    : fb(nullptr), allocator(nullptr), grid(nullptr), bg(Color::BLACK),
      inner({0, 0}), max_cols(0), max_rows(0), cursor(0, 0), idx(0), kstate(nullptr) {
        wipe_buffer();
    }

    void init(FrameBuffer* framebuffer, PhysicalMemoryAllocator* alloc, Point inner_origin, uint32_t cols, uint32_t rows, Color background, kernel_state* kernelState) {
        fb        = framebuffer;
        allocator = alloc;
        inner     = inner_origin;
        max_cols  = cols;
        max_rows  = rows;
        bg        = background;
        kstate    = kernelState;

        grid = (char*)allocator->malloc(grid_size());
        wipe_grid();

        cursor.reset();
        wipe_buffer();

        print_banner();
    }

    ~ShellApplication() {
        if (grid && allocator) {
            allocator->free(grid, grid_size());
            grid = nullptr;
        }
    }
    // needs this
    void handle_char(char ch) {
        erase_cursor();

        switch (ch) {
            case '\n':
                cursor.newline(max_rows);
                process_buffer();
                wipe_buffer();
                echo_char('>');
                cursor.move_right(max_cols);
                break;

            case '\b':
                if (cursor.get_col() > 1) {
                    cursor.move_left();
                    cell_at_cursor() = 0;
                    erase_cell();
                    if (idx > 0) { idx--; buf[idx] = 0; }
                }
                break;

            default:
                if (idx >= MAX_BUFFER_SIZE - 1) break;
                buf[idx++] = ch;
                cell_at_cursor() = ch;
                echo_char(ch);
                cursor.move_right(max_cols);
                break;
        }

        draw_cursor();
    }
    
    // needs this for any redraws to restore state
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

private:
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
        for (uint32_t i = 0; i < MAX_BUFFER_SIZE; i++)
            buf[i] = 0;
        idx = 0;
    }

    void process_buffer() {

    }

    void print_char_to_grid(char ch) {
        if (ch == '\n') {
            cursor.newline(max_rows);
            return;
        }
        cell_at_cursor() = ch;
        cursor.move_right(max_cols);
    }

    void print_string(const char* str) {
        for (const char* p = str; *p; p++) print_char_to_grid(*p);
    }

    void print_banner() {
        if (!kstate) return;

        print_string(kstate->kernel_name);
        print_string(" v");
        print_string(kstate->kernel_version_major);
        print_string(".");
        print_string(kstate->kernel_version_minor);
        print_char_to_grid('\n');

        print_string("Booted: ");
        print_string(kstate->boot_time);
        print_char_to_grid('\n');
        print_char_to_grid('\n');
        print_char_to_grid('>');
    }

    FrameBuffer*             fb;
    PhysicalMemoryAllocator* allocator;
    char*                    grid;   // max_rows * max_cols cells; the record
    Color                    bg;
    Point                    inner;  // top-left pixel of the content region
    uint32_t                 max_cols;
    uint32_t                 max_rows;
    TextCursor               cursor;
    char                     buf[MAX_BUFFER_SIZE];
    uint32_t                 idx;
    kernel_state*            kstate;
};

class WindowManager;

class Window {
private:
    friend class WindowManager;

    Window() : active(false), fb(nullptr), title(nullptr), bg(Color::BLACK),
               dims({0, 0}), origin({0, 0}), kstate(nullptr) {}

    Window(FrameBuffer* fb, PhysicalMemoryAllocator* allocator, Point origin,
           Dimensions dims, Color bg, const char* name, kernel_state* kstate, bool active = false)
    : active(active), fb(fb), title(name), bg(bg), dims(dims), origin(origin), kstate(kstate) {
        app.init(fb, allocator, inner_origin(), inner_width() / 8, inner_height() / 16, bg, kstate);
    }

public:
    void draw() {
        fb->draw_rectangle(origin, dims, bg);
        draw_borders();
        draw_title();
        draw_corner_dot();
        app.redraw();
        if (active) {
            app.draw_cursor();
        }
    }

    Point inner_origin() const { return { origin.x + side_border, origin.y + top_border }; }
    uint32_t inner_width()  const { return dims.width  - 2 * side_border; }
    uint32_t inner_height() const { return dims.height - top_border - side_border; }

    uint32_t get_corner_dot_radius() { return corner_dot_radius; }
    Point get_corner_dot_point() { return corner_dot_center; }

    void resolve_scancode(uint8_t scancode) {
        if (scancode & 0x80) return; // ignore key releases

        char ch;
        switch (scancode) {
            case 0x1C: ch = '\n'; break; // Enter
            case 0x0E: ch = '\b'; break; // Backspace
            default:
                ch = scancode_to_char(scancode);
                if (ch == 0) return; // no printable mapping
                break;
        }

        app.handle_char(ch);
    }

    void erase(Color desktop_bg) {
        fb->draw_rectangle(origin, dims, desktop_bg);
    }

    struct Dimensions get_dims() { return dims; }
    struct Point get_origin()    { return origin; }

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
        corner_dot_radius = radius;
        Point center = { origin.x + dims.width - side_border - radius - 4, origin.y + top_border / 2 };
        corner_dot_center = center;
        fb->draw_circle(center, radius, Color::RED);
    }

    bool             active;
    FrameBuffer*     fb;
    const char*      title;
    Color            bg;
    uint32_t         top_border  = 20;
    uint32_t         side_border = 5;
    Dimensions       dims;
    Point            origin;
    ShellApplication app;            // owns grid, buffer, cursor, all text state
    struct Point     corner_dot_center;
    uint32_t         corner_dot_radius;
    kernel_state*    kstate;
};

#define MAXIMUM_NUMBER_WINDOWS 4

class WindowManager {
public:

    WindowManager() {}

    WindowManager(FrameBuffer* fb, Color desktop_bg, PhysicalMemoryAllocator* allocator, kernel_state* kstate)
    : mouse(fb->get_width(), fb->get_height()), fb(fb), allocator(allocator),
      kstate(kstate), numWindows(0), desktop_bg(desktop_bg) {
        mouse.init();
        children = (Window**)allocator->malloc(sizeof(Window*) * MAXIMUM_NUMBER_WINDOWS);
        back_buffer = (uint64_t*)allocator->malloc(fb->get_size());
        fb->set_buffer_to_back(back_buffer);
    }


    uint32_t create_window(Point origin, Dimensions dims, Color bg, const char* name) {
        if (numWindows >= MAXIMUM_NUMBER_WINDOWS) return -1;
        bool active = (numWindows == 0); // first window is active by default

        void* mem = allocator->malloc(sizeof(Window));
        if (!mem) return -1;

        Window* win = new (mem) Window(fb, allocator, origin, dims, bg, name, kstate, active);
        children[numWindows] = win;
        return numWindows++;
    }

    void kill_window(uint32_t id) {
        if (numWindows == 0 || id >= numWindows) return;

        children[id]->active = false;
        children[id]->erase(desktop_bg);

        // Destructor first (frees the app's grid), then release the Window
        // memory itself.
        children[id]->~Window();
        allocator->free(children[id], sizeof(Window));

        for (uint32_t i = id; i < numWindows - 1; i++)
            children[i] = children[i + 1];
        children[numWindows - 1] = nullptr;
        numWindows--;

        if (numWindows == 0) {
            active_window_id = 0;
            return;
        }

        if (id < active_window_id)
            active_window_id--;
        else if (id == active_window_id && active_window_id >= numWindows)
            active_window_id = numWindows - 1;

        children[active_window_id]->active = true;
    }

    void draw() {
        
        for (uint32_t i = 0; i < numWindows; i++){
            drivers::serial::print(children[i]->get_dims().width);
            children[i]->draw();
        }
        fb->copy_back_buffer(back_buffer);
    }

    void change_active(uint8_t next_active) {
        if (next_active == active_window_id) return;

        children[active_window_id]->active = false;
        children[active_window_id]->app.erase_cursor();

        children[next_active]->active = true;
        children[next_active]->app.draw_cursor();
        active_window_id = next_active;
    }

    bool read_packet() {
        return mouse.read_packet();
    }

    void redraw_mouse() {

        bool left_clicked = mouse.left_clicked();
        struct Point point = mouse.get_old_position();

        if (left_clicked) {

            if (numWindows > 0) {
                Window* win = children[active_window_id];
                struct Point center = win->get_corner_dot_point();
                uint32_t radius = win->get_corner_dot_radius();

                if (point.x >= center.x - radius && point.x <= center.x + radius
                    && point.y >= center.y - radius && point.y <= center.y + radius) {
                        kill_window(active_window_id);
                        drivers::serial::print("Closed tab\n");
                    }
            }

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

        fb->copy_back_buffer(back_buffer);
        fb->set_buffer_to_original();

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

                if (kind == 1)
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, 0x00, 0x00, 0x00);
                else
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, 0xFF, 0xFF, 0xFF);
            }
        }

        fb->set_buffer_to_back(back_buffer);
    }

    void process_scancode(uint8_t code) {
        if (numWindows > 0){
            children[active_window_id]->resolve_scancode(code);
            fb->copy_back_buffer(back_buffer);
        }
    }

private:
    Mouse mouse;
    FrameBuffer* fb;
    PhysicalMemoryAllocator* allocator;
    kernel_state* kstate;
    uint32_t numWindows;
    Window** children; // array of window pointers
    uint64_t* back_buffer;
    Color desktop_bg;
    uint8_t active_window_id = 0;
};
