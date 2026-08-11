#pragma once
#include <stdint.h>
#include "new.h"
#include "util.h"
#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "drivers/serial.h"
#include "memory/allocator.h"
#include "nstd/managed_ptr.h"

#include "apps/nanoterm.h"
#include "kstate.h"
#include "kapi.h"

#define TITLE_COLOR       Color::BLACK
#define BORDER_COLOR      Color::SILVER

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

#define NAVBAR_OPTION_LOGO_HEIGHT 35
#define NAVBAR_OPTION_LOGO_HEIGHT 35

static const uint8_t terminal_logo[NAVBAR_OPTION_LOGO_HEIGHT][NAVBAR_OPTION_LOGO_HEIGHT] = {
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
};

static const uint8_t clock_logo[NAVBAR_OPTION_LOGO_HEIGHT][NAVBAR_OPTION_LOGO_HEIGHT] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0,0},
    {0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0},
    {0,0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0},
    {0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0},
    {0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,2,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,2,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0},
    {0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0},
    {0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0},
    {0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0,0},
    {0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0},
    {0,0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t settings_logo[NAVBAR_OPTION_LOGO_HEIGHT][NAVBAR_OPTION_LOGO_HEIGHT] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,1,1,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,1,1,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,0,0,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t manual_logo[NAVBAR_OPTION_LOGO_HEIGHT][NAVBAR_OPTION_LOGO_HEIGHT] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,1,2,1,1,1,1,1,1,1,1,1,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,1,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

static const uint8_t filesystem_logo[NAVBAR_OPTION_LOGO_HEIGHT][NAVBAR_OPTION_LOGO_HEIGHT] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};

class WindowManager;

class Window {
public:

    Window(FrameBuffer* fb, Point origin, Dimensions dims, Color bg, const char* name, kernel_state* kstate, kernel_api* kapi, bool active = false)
    : active(active), fb(fb), title(name), bg(bg), dims(dims), origin(origin), kstate(kstate), kapi(kapi) {
        app = nstd::make_managed<Application, NanoTerm>(fb, inner_origin(), inner_width() / 8, inner_height() / 16, bg, kstate, kapi);
    }
    
    void draw() {
        fb->draw_rectangle(origin, dims, bg);
        draw_borders();
        draw_title();
        draw_corner_dot();
        app->redraw();
        if (active) {
            app->draw_cursor();
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

        app->handle_char(ch);
    }

    void erase(Color desktop_bg) {
        fb->draw_rectangle(origin, dims, desktop_bg);
    }

    void move(int32_t dx, int32_t dy) {
        origin.x = (uint32_t)((int32_t)origin.x + dx);
        origin.y = (uint32_t)((int32_t)origin.y + dy);
        app->set_inner(inner_origin());
    }

    bool in_title_bar(Point p) const {
        return p.x >= origin.x && p.x <= origin.x + dims.width && p.y >= origin.y && p.y <= origin.y + top_border;
    }

    bool contains(Point p) const {
        return p.x >= origin.x && p.x <= origin.x + dims.width && p.y >= origin.y && p.y <= origin.y + dims.height;
    }

    bool in_close_button(Point p) const {
        Point c = corner_dot_center;
        uint32_t r = corner_dot_radius;
        return p.x >= c.x - r && p.x <= c.x + r && p.y >= c.y - r && p.y <= c.y + r;
    }

    struct Dimensions get_dims() { return dims; }
    struct Point get_origin()    { return origin; }

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
    nstd::managed_ptr<Application> app;
    uint32_t         corner_dot_radius;
    Point            corner_dot_center;
    kernel_state*    kstate;
    kernel_api*      kapi;
};

#define MAXIMUM_NUMBER_WINDOWS 4

#define NAVBAR_BG Color::WHITE
#define NAVBAR_OPTIONS 5
#define NAVBAR_HEIGHT 45

class WindowManager : public MouseObserver {
public:

    WindowManager(FrameBuffer* fb, Color desktop_bg, kernel_state* kstate, kernel_api* kapi)
    : fb(fb), kstate(kstate), kapi(kapi), numWindows(0), desktop_bg(desktop_bg) {
        
        drivers::serial::print("Acquiring back buffer memory\n");
        back_buffer = (uint64_t*)PhysicalMemoryAllocatorSingleton::getInstance().malloc(fb->get_size());
        fb->set_buffer_to_back(back_buffer);
        navbar_options[0] = "NanoTerm";
        navbar_options[1] = "Clock";
        navbar_options[2] = "Settings";
        navbar_options[3] = "Files";
        navbar_options[4] = "User Manual";
    }

    ~WindowManager(){
        drivers::serial::print("Freeing back buffer memory\n");
        PhysicalMemoryAllocatorSingleton::getInstance().free(back_buffer, fb->get_size());
    }

    void update(struct MouseState state) override {
        Point pos = { (uint32_t)state.x, (uint32_t)state.y };
        bool left = state.left_button_pressed;

        process_drag(pos, left);
        if (left) process_click(prev_pos);
        draw_cursor_overlay(pos);

        wasLeftPressed = left;
        prev_pos = pos;

    }

    uint32_t create_window(Point origin, Dimensions dims, Color bg, const char* name) {
        if (numWindows >= MAXIMUM_NUMBER_WINDOWS) return -1;

        if (numWindows > 0) {
            children[active_window_id]->active = false;
            children[active_window_id]->app->erase_cursor();
        }

        children[numWindows] = nstd::make_managed<Window>(fb, origin, dims, bg, name, kstate, kapi, true);
        active_window_id = numWindows;
        return numWindows++;
    }

    void kill_window(uint32_t id) {
        if (numWindows == 0 || id >= numWindows) return;

        children[id]->active = false;
        children[id]->erase(desktop_bg);

        children[id].reset();

        for (uint32_t i = id; i < numWindows - 1; i++)
            children[i] = nstd::move(children[i + 1]);
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

        draw_navbar();
        for (uint32_t i = 0; i < numWindows; i++){
            children[i]->draw();
        }


        fb->copy_back_buffer(back_buffer);
    }


    void change_active(uint8_t next_active) {
        if (next_active == active_window_id) return;

        children[active_window_id]->active = false;
        children[active_window_id]->app->erase_cursor();

        children[next_active]->active = true;
        children[next_active]->app->draw_cursor();
        active_window_id = next_active;
    }

    void process_scancode(uint8_t code) {
        if (numWindows > 0){
            children[active_window_id]->resolve_scancode(code);
            fb->copy_back_buffer(back_buffer);
        }
    }

private:
    void process_drag(Point pos, bool left) {
        if (left && !wasLeftPressed)      { dragging = true;  drag_start = pos; }
        else if (!left && wasLeftPressed) { dragging = false; drag_end   = pos; }

        if (wasDragging && !dragging && numWindows > 0) {
            Window* win = children[active_window_id].get();
            if (win->in_title_bar(drag_start)) {
                win->erase(desktop_bg);
                win->move((int32_t)drag_end.x - (int32_t)drag_start.x, (int32_t)drag_end.y - (int32_t)drag_start.y);
                win->draw();
            }
        }
        wasDragging = dragging;
    }

    void draw_navbar(){
        fb->draw_rectangle({0, fb->get_height() - NAVBAR_HEIGHT}, {fb->get_width(), NAVBAR_HEIGHT}, NAVBAR_BG);
        uint32_t space_per_item = fb->get_width() / NAVBAR_OPTIONS;

        uint32_t bottom_y = fb->get_height();
        uint32_t top_y = fb->get_height() - NAVBAR_HEIGHT;
        uint32_t middle_y = (bottom_y + top_y) / 2;

        for (int i = 0; i < NAVBAR_OPTIONS; i++){
            uint32_t left_x = space_per_item * i;
            uint32_t right_x = space_per_item * (i + 1);
            uint32_t middle_x = (left_x + right_x) / 2;

            const uint8_t (*logo)[NAVBAR_OPTION_LOGO_HEIGHT];
            switch (i) {
                case 0:  logo = terminal_logo; break;
                case 1:  logo = clock_logo;    break;
                case 2:  logo = settings_logo; break;
                case 3:  logo = filesystem_logo; break;
                case 4:  logo = manual_logo;   break;
                default: logo = clock_logo;    break;
            }

            for (int32_t row = 0; row < NAVBAR_OPTION_LOGO_HEIGHT; row++) {
                for (int32_t col = 0; col < NAVBAR_OPTION_LOGO_HEIGHT; col++) {
                    uint8_t kind = logo[row][col];
                    if (kind == 0) continue;
                    int32_t x = (int32_t)middle_x - (35 / 2)+ col;
                    int32_t y = (int32_t)middle_y - (35 / 2) + row;
                    if (x < 0 || y < 0) continue;
                    if ((uint32_t)x >= fb->get_width() || (uint32_t)y >= fb->get_height()) continue;

                    uint8_t v = (kind == 1) ? 0x00 : 0xFF;
                    fb->set_pixel({(uint32_t)x, (uint32_t)y}, v, v, v);
                }
            }
        }
    }

    void process_click(Point at) {
        if (numWindows == 0) return;

        uint32_t bottom_y = fb->get_height();
        uint32_t top_y = fb->get_height() - NAVBAR_HEIGHT;

        if (at.y >= top_y && at.y < bottom_y) {
            uint32_t space_per_item = fb->get_width() / NAVBAR_OPTIONS;
            uint32_t option_index = at.x / space_per_item;
            if (option_index < NAVBAR_OPTIONS) {
                switch (option_index) {
                    case 0:  create_window({100, 100}, {400, 300}, Color::BLACK, "NanoTerm"); break;
                    case 1:  create_window({150, 150}, {300, 200}, Color::BLACK, "Clock"); break;
                    case 2:  create_window({200, 200}, {350, 250}, Color::BLACK, "Settings"); break;
                    case 3:  create_window({250, 250}, {400, 300}, Color::BLACK, "Files"); break;
                    case 4:  create_window({300, 300}, {450, 350}, Color::BLACK, "User Manual"); break;
                }
                draw();
                return;
            }
        }

        if (children[active_window_id]->in_close_button(at)) {
            kill_window(active_window_id);
            return;
        }

        for (uint32_t i = 0; i < numWindows; i++) {
            if (children[i]->contains(at)) { change_active(i); break; }
        }
    }

    void draw_cursor_overlay(Point pos) {
        fb->copy_back_buffer(back_buffer);
        fb->set_buffer_to_original();

        for (int32_t row = 0; row < CURSOR_HEIGHT; row++) {
            for (int32_t col = 0; col < CURSOR_WIDTH; col++) {
                uint8_t kind = retro_cursor_bitmap[row][col];
                if (kind == 0) continue;
                int32_t x = (int32_t)pos.x + col;
                int32_t y = (int32_t)pos.y + row;
                if (x < 0 || y < 0) continue;
                if ((uint32_t)x >= fb->get_width() || (uint32_t)y >= fb->get_height()) continue;

                uint8_t v = (kind == 1) ? 0x00 : 0xFF;
                fb->set_pixel({(uint32_t)x, (uint32_t)y}, v, v, v);
            }
        }

        fb->set_buffer_to_back(back_buffer);
    }

    bool wasLeftPressed = false;
    bool wasDragging = false;
    bool dragging = false;
    Point drag_start{0, 0};
    Point drag_end{0, 0};
    Point prev_pos{0, 0};   // mouse position from the previous packet

    char* navbar_options[NAVBAR_OPTIONS];

    FrameBuffer* fb;
    kernel_state* kstate;
    kernel_api* kapi;

    uint32_t numWindows;
    nstd::managed_ptr<Window> children[MAXIMUM_NUMBER_WINDOWS];
    uint64_t* back_buffer;
    Color desktop_bg;
    uint8_t active_window_id = 0;
};
