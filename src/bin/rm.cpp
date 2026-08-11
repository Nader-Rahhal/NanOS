#include <stdint.h>

#include "kstate.h"
#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi, struct kernel_state* kstate) {
    if (argc < 2) {
        kapi->puts("Usage: rm <filename>\n");
        return;
    }
    kapi->puts("Removing ");
    kapi->puts(argv[1]);
    kapi->puts("\n");
    kapi->rm(argv[1]);
}
