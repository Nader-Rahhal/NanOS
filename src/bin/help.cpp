#include <stdint.h>

#include "kstate.h"
#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi, struct kernel_state* kstate) {
    kapi->puts("Available commands:\n");
    kapi->puts("  help        - show this help\n");
    kapi->puts("  version     - show kernel version\n");
    kapi->puts("  ls          - list files\n");
    kapi->puts("  touch <f>   - create file <f>\n");
    kapi->puts("  rm <f>      - remove file <f>\n");
}
