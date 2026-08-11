#pragma once

#include "graphics/application.h"

class NanoTerm : public Application {
public:

    NanoTerm(FrameBuffer* framebuffer, Point inner_origin, uint32_t cols, uint32_t rows, Color background, kernel_state* kernelState, kernel_api* kernelApi){
        fb  = framebuffer;
        inner = inner_origin;
        max_cols = cols;
        max_rows = rows;
        bg = background;
        kstate = kernelState;
        kapi = kernelApi;

        grid = (char*)PhysicalMemoryAllocatorSingleton::getInstance().malloc(grid_size());
        wipe_grid();

        cursor.reset();
        wipe_buffer();

        print_banner();
    }

    ~NanoTerm() {
        if (grid) {
            PhysicalMemoryAllocatorSingleton::getInstance().free(grid, grid_size());
            grid = nullptr;
        }
    }
    // needs this
    void handle_char(char ch) override {
        erase_cursor();

        switch (ch) {
            case '\n':
                cursor.newline(max_rows);
                process_buffer();
                wipe_buffer();
                cell_at_cursor() = '>';
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

private:

    void process_buffer() {
        if (!kapi || !kapi->exec || buf[0] == '\0') return;

        char* argv[8];
        int argc = 0;
        char* p = buf.get();
        while (*p && argc < 8) {
            while (*p == ' ') *p++ = '\0';
            if (*p == '\0') break;
            argv[argc++] = p;
            while (*p && *p != ' ') p++;
        }

        if (argc == 0) return;
        bool success = kapi->exec(argc, argv);
        if (success) {
            if (kapi->out_len > 0) print_output(kapi->out);
        } else {
            print_output("Error: failed to execute '");
            print_output(argv[0]);
            print_output("'\n");
        }
    }

    void print_char_to_grid(char ch) {
        if (ch == '\n') {
            cursor.newline(max_rows);
            return;
        }
        cell_at_cursor() = ch;
        cursor.move_right(max_cols);
    }

    void print_output(const char* str) {
        for (const char* p = str; *p; p++) {
            if (*p == '\r') continue;
            if (*p == '\n') {
                cursor.newline(max_rows);
                continue;
            }
            cell_at_cursor() = *p;
            echo_char(*p);
            cursor.move_right(max_cols);
        }
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
};