#include <stdint.h>

#include "kstate.h"
#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi, struct kernel_state* kstate) {
    kapi->puts("Listing files...\n");
    char names[64][256];
    uint32_t n = kapi->ls(names, 64);
    for (uint32_t i = 0; i < n; i++){
        kapi->puts(names[i]);
        kapi->puts("\n");
    }
}