
#include "graphics/window.h"
#include "elf/elf.h"
#include "memory/mmap.h"
#include "acpi/acpi.h"
#include "arch/apic.h"
#include "arch/gdt.h"
#include "arch/idt.h"
#include "arch/pic.h"
#include "arch/exception.h"
#include "fs/ext2.h"

#include "nstd/ring_buffer.h"

extern "C" { extern uint8_t _binary_fonts_default_psf_start[]; }


struct KernelServices {
    WindowManager* window_manager;
    Mouse* mouse;
};

struct KernelServices kservices;
struct KernelParams* kparams;

extern "C" void interrupt_handler(arch::idt::InterruptFrame* frame) {
    if (frame->vector < 32) {
        arch::exception::exception_handler(frame);
        return;
    }

    if (frame->vector == 255) {
        return;
    }

    if (frame->vector == 33) {
        uint8_t scancode = util::inb(0x60);
        kservices.window_manager->process_scancode(scancode);
    }

    if (frame->vector == 44 && kservices.mouse != nullptr) {
        kservices.mouse->read_packet();
    }

    arch::apic::lapic_eoi();
}

struct KernelParams {
    FrameBuffer* fb;
    MMap* mm;
    acpi::MADT* madt;
    kernel_state* kstate;
};

#define DESKTOP_BG Color::DARK_SLATE_GRAY

#define MAX_PROGRAM_SIZE (64 * 1024)

uint8_t elf_buf[MAX_PROGRAM_SIZE];

bool try_exec(int argc, char** argv);   
void kern_puts(const char* s);

struct kernel_api g_kapi = {
    .puts = kern_puts,
    .touch = fs::ext2::create_file,
    .exec = try_exec,
    .ls = fs::ext2::list_entries,
    .rm = fs::ext2::delete_file,
};

void kern_puts(const char* s){
    util::serial_puts(s);
    while (*s && g_kapi.out_len < KAPI_OUT_CAP - 1){
        g_kapi.out[g_kapi.out_len++] = *s++;
    }
    g_kapi.out[g_kapi.out_len] = '\0';
}

bool try_exec(int argc, char** argv){

    (void)argc;

    char path[80];
    const char* prefix = "bin/";
    uint32_t plen = util::strlen(prefix);
    uint32_t alen = util::strlen(argv[0]);
    if (plen + alen >= sizeof(path)) return false;

    util::memcpy(path, prefix, plen);
    util::memcpy(path + plen, argv[0], alen);
    path[plen + alen] = '\0';

    int64_t res = fs::ext2::read_file(path, elf_buf, MAX_PROGRAM_SIZE);

    if (res < 0) {
        return false;
    }

    uint64_t entry = elf::load(elf_buf);

    g_kapi.out_len = 0;
    g_kapi.out[0] = '\0';

    typedef void (*program_entry_t)(int argc, char** argv, kernel_api* kapi, kernel_state* kstate);
    ((program_entry_t)entry)(argc, argv, &g_kapi, kparams->kstate);
    return true;
}

extern "C" __attribute__((section(".text.kmain"))) void kmain(struct KernelParams* params)
{
    kparams = params;
    drivers::serial::init();

    PhysicalMemoryAllocatorSingleton::getInstance().setValue(params->mm);

    arch::pic::disable();

    params->fb->set_font(_binary_fonts_default_psf_start);

    arch::gdt::init();
    drivers::serial::print("GDT loaded\r\n");

    arch::idt::init();
    drivers::serial::print("IDT loaded\r\n");

    arch::apic::init(params->madt);
    drivers::serial::print("APIC initialized\r\n");

    arch::idt::enable_interrupts();
    drivers::serial::print("Interrupts enabled\r\n");
    fs::ext2::parse_superblock();

    Mouse mouse(params->fb->get_width(), params->fb->get_height());
    mouse.init();

    WindowManager manager(params->fb, DESKTOP_BG, kparams->kstate, &g_kapi);
    kservices.window_manager = &manager;
    params->fb->set_background(DESKTOP_BG);

    kservices.window_manager->create_window({0, 0}, {600, 400}, Color::DARK_GRAY, "NanoTerm");
    kservices.window_manager->draw();

    mouse.attach(&manager);
    kservices.mouse = &mouse;

    nstd::ring_buffer<int, 10> buf;
    for (int i = 0; i < 20; i++){
        buf.insert(i);
    }

    for (;;) {
        __asm__ volatile ("hlt");
    }
}