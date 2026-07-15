#pragma once
#include <stdint.h>
#include "acpi/acpi.h"

#define LAPIC_ID   0x020
#define LAPIC_TPR  0x080
#define LAPIC_EOI  0x0B0
#define LAPIC_SVR  0x0F0

namespace arch::apic {

namespace detail {

struct __attribute__((packed)) IOAPICRecord {
    uint8_t  type;      // 1
    uint8_t  length;    // 12
    uint8_t  id;
    uint8_t  reserved;
    uint32_t address;   // MMIO base
    uint32_t gsi_base;  // first GSI this I/O APIC handles
};

struct __attribute__((packed)) ISORecord {
    uint8_t  type;   // 2
    uint8_t  length; // 10
    uint8_t  bus;    // 0 = ISA
    uint8_t  irq;    // ISA IRQ number
    uint32_t gsi;    // GSI it maps to
    uint16_t flags;  // bits[1:0]=polarity, bits[3:2]=trigger mode
};

static volatile uint32_t* lapic_ptr;

static inline uint32_t lapic_read(uint32_t reg) {
    return lapic_ptr[reg >> 2];
}

static inline void lapic_write(uint32_t reg, uint32_t val) {
    lapic_ptr[reg >> 2] = val;
}

static inline uint32_t ioapic_read(volatile uint32_t* base, uint8_t reg) {
    base[0] = reg;
    return base[4];
}

static inline void ioapic_write(volatile uint32_t* base, uint8_t reg, uint32_t val) {
    base[0] = reg;
    base[4] = val;
}

static void ioapic_redirect(volatile uint32_t* ioapic, uint32_t slot, uint8_t vector,
                            uint8_t dest, bool active_low, bool level) {
    uint32_t lo = (uint32_t)vector
                | (active_low ? (1u << 13) : 0u)
                | (level      ? (1u << 15) : 0u);
    ioapic_write(ioapic, 0x10 + 2 * slot, lo);
    ioapic_write(ioapic, 0x11 + 2 * slot, (uint32_t)dest << 24);
}

}

inline void lapic_eoi() {
    detail::lapic_write(LAPIC_EOI, 0);
}

inline void init(acpi::MADT* madt) {

    uint32_t irq_to_gsi[16];
    bool     active_low[16];
    bool     level_trig[16];
    for (int i = 0; i < 16; i++) {
        irq_to_gsi[i] = i;
        active_low[i] = false;
        level_trig[i] = false;
    }

    volatile uint32_t* ioapic = nullptr;
    uint32_t gsi_base = 0;

    uint8_t* ptr = (uint8_t*)madt + sizeof(acpi::MADT);
    uint8_t* end = (uint8_t*)madt + madt->Header.Length;

    while (ptr < end) {
        uint8_t type = ptr[0], len = ptr[1];
        if (type == 1) {
            auto* r  = (detail::IOAPICRecord*)ptr;
            ioapic   = (volatile uint32_t*)(uintptr_t)r->address;
            gsi_base = r->gsi_base;
        } else if (type == 2) {
            auto* r = (detail::ISORecord*)ptr;
            if (r->bus == 0 && r->irq < 16) {
                irq_to_gsi[r->irq] = r->gsi;
                active_low[r->irq] = (r->flags & 0x3) == 0x3;
                level_trig[r->irq] = ((r->flags >> 2) & 0x3) == 0x3;
            }
        }
        ptr += len;
    }

    detail::lapic_ptr = (volatile uint32_t*)(uintptr_t)madt->LocalApicAddress;
    detail::lapic_write(LAPIC_TPR, 0);
    detail::lapic_write(LAPIC_SVR, 0x1FF);
    uint8_t lapic_id = (detail::lapic_read(LAPIC_ID) >> 24) & 0xFF;

    // route IRQ 1 (PS/2 keyboard) to vector 33
    detail::ioapic_redirect(ioapic, irq_to_gsi[1]  - gsi_base, 33, lapic_id, active_low[1],  level_trig[1]);

    // route IRQ 12 (PS/2 mouse) to vector 44
    detail::ioapic_redirect(ioapic, irq_to_gsi[12] - gsi_base, 44, lapic_id, active_low[12], level_trig[12]);
}

}
