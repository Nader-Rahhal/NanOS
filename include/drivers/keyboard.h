#pragma once
static const char kbd_lower[128] = {
    0,    0,   '1', '2', '3', '4', '5', '6',   // 0x00-0x07
    '7',  '8', '9', '0', '-', '=', '\b', '\t', // 0x08-0x0F
    'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',   // 0x10-0x17
    'o',  'p', '[', ']', '\n', 0, 'a', 's',    // 0x18-0x1F  0x1D=LCtrl
    'd',  'f', 'g', 'h', 'j', 'k', 'l', ';',   // 0x20-0x27
    '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v',   // 0x28-0x2F  0x2A=LShift
    'b',  'n', 'm', ',', '.', '/', 0,   '*',   // 0x30-0x37  0x36=RShift
    0,    ' ', 0,   0,   0,   0,   0,   0,      // 0x38-0x3F  0x38=LAlt 0x3A=Caps
};

static const char kbd_upper[128] = {
    0,    0,   '!', '@', '#', '$', '%', '^',
    '&',  '*', '(', ')', '_', '+', '\b', '\t',
    'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O',  'P', '{', '}', '\n', 0, 'A', 'S',
    'D',  'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"',  '~', 0,  '|', 'Z', 'X', 'C', 'V',
    'B',  'N', 'M', '<', '>', '?', 0,   '*',
    0,    ' ', 0,   0,   0,   0,   0,   0,
};

char scancode_to_char(uint8_t sc) {
    static bool shift = false;
    static bool caps  = false;

    if (sc & 0x80) {
        uint8_t make = sc & 0x7F;
        if (make == 0x2A || make == 0x36) shift = false;
        return 0;
    }

    if (sc == 0x2A || sc == 0x36) { shift = true;  return 0; } // L/R shift
    if (sc == 0x3A)               { caps = !caps;  return 0; } // caps lock

    char c = shift ? kbd_upper[sc] : kbd_lower[sc];
    if (c == 0) return 0;

    if (caps && c >= 'a' && c <= 'z') c -= 32;
    else if (caps && shift && c >= 'A' && c <= 'Z') c += 32;

    return c;
}