#include <stdint.h>

#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi) {
    kapi->puts("Creating file...\n");
    if (argc < 2) {
        kapi->puts("Usage: touch <filename>\n");
        return;
    }
    kapi->touch(argv[1]);
}