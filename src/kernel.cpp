#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "graphics/window.h"

#include "elf.h"
#include "memory/mmap.h"
#include "acpi/acpi.h"
#include "arch/apic.h"
#include "util.h"
#include "memory/allocator.h"
#include "arch/gdt.h"
#include "arch/idt.h"
#include "panic.h"
#include "arch/pic.h"
#include "arch/exception.h"
#include "drivers/serial.h"
#include "drivers/ata.h"
#include "fs/ext2.h"

#include "kapi.h"

extern "C" { extern uint8_t _binary_fonts_default_psf_start[]; }


struct KernelServices {
    PhysicalMemoryAllocator* allocator;
    WindowManager* window_manager;
};

struct KernelServices kservices;

extern "C" void interrupt_handler(arch::idt::InterruptFrame* frame) {
    if (frame->vector < 32) {
        arch::exception::exception_handler(frame);
        return;
    }

    if (frame->vector == 255) {
        // LAPIC spurious interrupt do not send EOI
        return;
    }

    if (frame->vector == 33) {
        uint8_t scancode = util::inb(0x60);
        kservices.window_manager->process_scancode(scancode);
    }

    if (frame->vector == 44 && kservices.window_manager != nullptr) {
        if (kservices.window_manager->read_packet())
            kservices.window_manager->redraw_mouse();
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

struct kernel_api g_kapi = {
    .puts = util::serial_puts,
    .touch = fs::ext2::create_file,
};

void try_exec(int argc, char** argv){
    (void)argc;
    const char* fname = argv[0];

    fs::ext2::read_file(fname, elf_buf, MAX_PROGRAM_SIZE);
    
    uint64_t entry = elf::load(elf_buf);

    typedef void (*program_entry_t)(int argc, char** argv, kernel_api* kapi);
    ((program_entry_t)entry)(argc, argv, &g_kapi);
}

extern "C" __attribute__((section(".text.kmain"))) void kmain(struct KernelParams* params)
{

    drivers::serial::init();

    PhysicalMemoryAllocator allocator(params->mm);
    kservices.allocator = &allocator;

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

    WindowManager manager(params->fb, DESKTOP_BG, kservices.allocator, params->kstate);
    kservices.window_manager = &manager;
    params->fb->set_background(DESKTOP_BG);

    drivers::serial::print("Framebuffer width:  "); 
    drivers::serial::print(params->fb->get_width());
    drivers::serial::print("\r\n");
    drivers::serial::print("Framebuffer height: "); 
    drivers::serial::print(params->fb->get_height());
    drivers::serial::print("\r\n");
    drivers::serial::print("Framebuffer size:   "); 
    drivers::serial::print(params->fb->get_size());
    drivers::serial::print("\r\n");
    

    kservices.window_manager->create_window({0, 0}, {600, 400}, Color::DARK_GRAY, "NanoTerm");
    // kservices.window_manager->create_window({700, 10}, {600, 400}, Color::DARK_GRAY, "NanoTerm");
    kservices.window_manager->draw();


    for (;;) {
        __asm__ volatile ("hlt");
    }
}