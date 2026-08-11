#include <stdint.h>

#include "kstate.h"
#include "kapi.h"

extern "C"  __attribute__((section(".text._main"))) void _main(int argc, char** argv, struct kernel_api* kapi, struct kernel_state* kstate) {
    kapi->puts(kstate->kernel_name);
    kapi->puts(" v");
    kapi->puts(kstate->kernel_version_major);
    kapi->puts(".");
    kapi->puts(kstate->kernel_version_minor);
    kapi->puts("\n");
}
