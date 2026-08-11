#include <stdint.h>

#include "kstate.h"
#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi, struct kernel_state* kstate) {
    kapi->puts("Booted at ");
    kapi->puts(kstate->boot_time);
    kapi->puts("\n");
}