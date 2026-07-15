#pragma once
#include "idt.h"
#include "panic.h"

namespace arch::exception::detail {

enum class ExceptionResult {
    PANIC,
    RESUME
};

static const char* exception_names[] = {
    "Divide Error",        "Debug",                "NMI",                   "Breakpoint",
    "Overflow",            "Bound Range",          "Invalid Opcode",        "Device Not Available",
    "Double Fault",        "Coprocessor Overrun",  "Invalid TSS",           "Segment Not Present",
    "Stack Fault",         "General Protection",   "Page Fault",            "Reserved",
    "x87 FP Error",        "Alignment Check",      "Machine Check",         "SIMD FP Error",
    "Virtualization",      "Control Protection",   "Reserved",              "Reserved",
    "Reserved",            "Reserved",             "Reserved",              "Reserved",
    "Hypervisor Injection","VMM Communication",    "Security",              "Reserved",
};

ExceptionResult handle_divide_error(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_debug(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_nmi(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_breakpoint(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_overflow(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_bound_range(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_invalid_opcode(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_device_not_available(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_double_fault(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_invalid_tss(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_segment_not_present(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_stack_fault(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_general_protection(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_page_fault(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_x87_fp_error(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_alignment_check(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_machine_check(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_simd_fp_error(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_virtualization(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_control_protection(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_hypervisor_injection(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_vmm_communication(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}
ExceptionResult handle_security(arch::idt::InterruptFrame* frame) {
    return ExceptionResult::PANIC;
}

ExceptionResult handle_exception(arch::idt::InterruptFrame* frame) {
    switch (frame->vector) {
        case 0:  return handle_divide_error(frame);
        case 1:  return handle_debug(frame);
        case 2:  return handle_nmi(frame);
        case 3:  return handle_breakpoint(frame);
        case 4:  return handle_overflow(frame);
        case 5:  return handle_bound_range(frame);
        case 6:  return handle_invalid_opcode(frame);
        case 7:  return handle_device_not_available(frame);
        case 8:  return handle_double_fault(frame);
        case 10: return handle_invalid_tss(frame);
        case 11: return handle_segment_not_present(frame);
        case 12: return handle_stack_fault(frame);
        case 13: return handle_general_protection(frame);
        case 14: return handle_page_fault(frame);
        case 16: return handle_x87_fp_error(frame);
        case 17: return handle_alignment_check(frame);
        case 18: return handle_machine_check(frame);
        case 19: return handle_simd_fp_error(frame);
        case 20: return handle_virtualization(frame);
        case 21: return handle_control_protection(frame);
        case 28: return handle_hypervisor_injection(frame);
        case 29: return handle_vmm_communication(frame);
        case 30: return handle_security(frame);
        default: return ExceptionResult::PANIC;
    }
}
}

namespace arch::exception {
void exception_handler(arch::idt::InterruptFrame* frame) {
    detail::ExceptionResult result = detail::handle_exception(frame);
    if (result == detail::ExceptionResult::PANIC)
        KPANIC(detail::exception_names[frame->vector]);
}
}