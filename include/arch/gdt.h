
#pragma once
#include <stdint.h>
#include "tss.h"

#define GDT_OFFSET_NULL        0x00
#define GDT_OFFSET_KERNEL_CODE 0x08
#define GDT_OFFSET_KERNEL_DATA 0x10
#define GDT_OFFSET_USER_CODE   0x18
#define GDT_OFFSET_USER_DATA   0x20
#define GDT_OFFSET_TSS         0x28

namespace arch::gdt {

namespace detail {

struct __attribute__((packed)) GDTR {
    uint16_t size; // size of GDT
    uint64_t offset; // linear address of GDT
};

struct __attribute__((packed)) SegmentDescriptor {
    uint16_t limit_low;   // Limit bits 15:0
    uint16_t base_low;    // Base bits 15:0
    uint8_t  base_mid;    // Base bits 23:16
    uint8_t  access;      // Access byte
    uint8_t  limit_flags; // Limit bits 19:16 (low nibble) + Flags (high nibble)
    uint8_t  base_high;   // Base bits 31:24
};

struct __attribute__((packed)) SystemSegmentDescriptor {
    uint16_t limit_low;   // Limit bits 15:0
    uint16_t base_low;    // Base bits 15:0
    uint8_t  base_mid;    // Base bits 23:16
    uint8_t  access;      // Access byte
    uint8_t  limit_flags; // Limit bits 19:16 (low nibble) + Flags (high nibble)
    uint8_t  base_high;   // Base bits 31:24
    uint32_t base_upper;  // Base bits 63:32
    uint32_t reserved;
};

struct __attribute__((packed)) GDT {
    SegmentDescriptor null_descriptor;
    SegmentDescriptor kernel_code_descriptor;
    SegmentDescriptor kernel_data_descriptor;
    SegmentDescriptor user_code_descriptor;
    SegmentDescriptor user_data_descriptor;
    SystemSegmentDescriptor tss_descriptor;
};

inline SegmentDescriptor create_descriptor(uint8_t access, uint8_t limit_flags){
    SegmentDescriptor segment;
    segment.limit_low = 0xFFFF;
    segment.base_low = 0x0000;
    segment.base_mid = 0x00;
    segment.access = access;
    segment.limit_flags = limit_flags;
    segment.base_high = 0x00;
    return segment;
}

inline SystemSegmentDescriptor create_system_descriptor(uint64_t base, uint32_t limit, uint8_t access) {
    SystemSegmentDescriptor desc;
    desc.limit_low   = limit & 0xFFFF;
    desc.base_low    = (base >>  0) & 0xFFFF;
    desc.base_mid    = (base >> 16) & 0xFF;
    desc.access      = access;
    desc.limit_flags = (limit >> 16) & 0x0F;
    desc.base_high   = (base >> 24) & 0xFF;
    desc.base_upper  = (base >> 32) & 0xFFFFFFFF;
    desc.reserved    = 0;
    return desc;
}

inline void load_gdt(GDTR* gdtr) {
    __asm__ volatile (
        "lgdt %0\n"
        "push $0x08\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        :
        : "m"(*gdtr)
        : "rax", "memory"
    );
}

} // namespace detail

inline void init() {
    static detail::GDT gdt;
    static arch::tss::TSS tss;

    gdt.null_descriptor        = detail::create_descriptor(0x00, 0x00);
    gdt.kernel_code_descriptor = detail::create_descriptor(0x9A, 0xAF);
    gdt.kernel_data_descriptor = detail::create_descriptor(0x92, 0xCF);
    gdt.user_code_descriptor   = detail::create_descriptor(0xFA, 0xAF);
    gdt.user_data_descriptor   = detail::create_descriptor(0xF2, 0xCF);
    gdt.tss_descriptor         = detail::create_system_descriptor((uint64_t)&tss, sizeof(tss) - 1, 0x89);

    detail::GDTR gdtr;
    gdtr.size   = sizeof(gdt) - 1;
    gdtr.offset = (uint64_t)&gdt;
    detail::load_gdt(&gdtr);

    __asm__ volatile ("ltr %0" :: "r"((uint16_t)GDT_OFFSET_TSS));
}

} // namespace arch::gdt
