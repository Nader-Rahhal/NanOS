#include "graphics/color.h"
#include "graphics/framebuffer.h"
#include "graphics/window.h"

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

extern "C" { extern uint8_t _binary_font_psf_start[]; }

struct KernelServices {
    PhysicalMemoryAllocator* allocator;
    WindowManager* window_manager;
};



struct KernelServices kservices;

extern "C" void interrupt_handler(InterruptFrame* frame) {
    if (frame->vector < 32) {
        exception_handler(frame);
        return;
    }

    if (frame->vector == 255) {
        // LAPIC spurious interrupt — do NOT send EOI
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

    lapic_eoi();
}

struct KernelParams {
    FrameBuffer* fb;
    MMap* mm;
    MADT* madt;
};

#define DESKTOP_BG Color::DARK_SLATE_GRAY

void test_ata() {
    ata::init();

    uint16_t ata_write_buf[BYTES_PER_SECTOR / 2];
    for (int i = 0; i < BYTES_PER_SECTOR / 2; i++) {
        ata_write_buf[i] = (uint16_t)i;
    }

    // LBA 1 to avoid clobbering a boot sector at LBA 0.
    ata::write_sector(1, ata_write_buf);

    uint16_t ata_read_buf[BYTES_PER_SECTOR / 2];
    ata::read_sector(1, ata_read_buf);

    bool ata_ok = true;
    for (int i = 0; i < BYTES_PER_SECTOR / 2; i++) {
        if (ata_read_buf[i] != ata_write_buf[i]) {
            ata_ok = false;
            break;
        }
    }
    serial::print(ata_ok ? "ATA read/write test passed\r\n" : "ATA read/write test FAILED\r\n");
}

void test_allocator(PhysicalMemoryAllocator& allocator) {
    void* a = allocator.malloc(PAGE_SIZE);
    serial::print("malloc(PAGE_SIZE) a = ");
    serial::print((uint64_t)a);
    serial::print(a != nullptr ? " (ok)\r\n" : " (FAIL)\r\n");

    void* b = allocator.malloc(4 * PAGE_SIZE);
    serial::print("malloc(4 * PAGE_SIZE) b = ");
    serial::print((uint64_t)b);
    serial::print(b != nullptr ? " (ok)\r\n" : " (FAIL)\r\n");

    allocator.free(a, PAGE_SIZE);
    void* c = allocator.malloc(PAGE_SIZE);
    serial::print("malloc after free c = ");
    serial::print((uint64_t)c);
    serial::print(c == a ? " (matches freed block, ok)\r\n" : " (mismatch)\r\n");

    allocator.free(b, 4 * PAGE_SIZE);
    allocator.free(c, PAGE_SIZE);
}

extern "C" __attribute__((section(".text.kmain"))) void kmain(struct KernelParams* params)
{

    serial::init();

    test_ata();

    PhysicalMemoryAllocator allocator(params->mm);

    kservices.allocator = &allocator;

    test_allocator(allocator);

    disable_pic();

    params->fb->set_background(DESKTOP_BG);
    params->fb->set_font(_binary_font_psf_start);

    gdt_init();
    serial::print("GDT loaded\r\n");

    idt_init();
    serial::print("IDT loaded\r\n");

    apic_init(params->madt);
    serial::print("APIC initialized\r\n");

    enable_interrupts();
    serial::print("Interrupts enabled\r\n");

    WindowManager manager(params->fb, DESKTOP_BG, kservices.allocator);
    kservices.window_manager = &manager;
    

    kservices.window_manager->create_window({10, 10}, {600, 400}, Color::DARK_GRAY, "NanoTerm");
    kservices.window_manager->create_window({700, 10}, {600, 400}, Color::DARK_GRAY, "NanoTerm");
    kservices.window_manager->draw();



    for (;;) {
        __asm__ volatile ("hlt");
    }
}