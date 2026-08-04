#pragma once
#include <stdint.h>

class TextCursor {
public:
    TextCursor() : col(0), row(0) {}
    TextCursor(uint32_t col, uint32_t row) : col(col), row(row), start_col(col), start_row(row) {}

    void move_left()                      { if (col > 0) col--; }
    void move_right(uint32_t max_cols)    { if (col + 1 < max_cols) col++; }
    void move_up()                        { if (row > 0) row--; }
    void move_down(uint32_t max_rows)     { if (row + 1 < max_rows) row++; }
    void newline(uint32_t max_rows)       { col = 0; if (row + 1 < max_rows) row++; }
    void carriage_return()                { col = 0; }

    uint32_t get_col() const { return col; }
    uint32_t get_row() const { return row; }

    void reset(){
        col = start_col;
        row = start_row;
    }



private:
    uint32_t start_col;
    uint32_t start_row;

    uint32_t col;
    uint32_t row;

    uint32_t start_col;
    uint32_t start_row;
};