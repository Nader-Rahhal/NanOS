#pragma once
#include <stdint.h>

#define KAPI_OUT_CAP 1024

struct kernel_api {
    void (*puts)(const char* s);
    int (*touch)(const char* name);
    bool (*exec)(int argc, char** argv);
    uint32_t (*ls)(char names[][256], uint32_t max_names);
    bool (*rm)(const char* name);


    char out[KAPI_OUT_CAP];
    uint32_t out_len;
};
