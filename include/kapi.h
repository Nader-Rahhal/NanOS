#pragma once

struct kernel_api {
    void (*puts)(const char* s);
    int (*touch)(const char* name);
};