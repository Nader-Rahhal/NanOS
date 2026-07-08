#pragma once

#include <stdint.h>

struct __attribute__((__packed__)) ELF_HEADER {
    uint32_t magic;
    uint8_t format;
    uint8_t endianess;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t reserved[7];
    uint16_t file_type;
    uint16_t isa;
    uint32_t version2;
    uint64_t entry_point;
    uint64_t ph_offset;
    uint64_t sh_offset;
    uint32_t flags;
    uint16_t eh_size;
    uint16_t ph_entry_size;
    uint16_t ph_entry_count;
    uint16_t sh_entry_size;
    uint16_t sh_entry_count;
    uint16_t sh_str_index;
};

struct __attribute__((__packed__)) ELF_PROGRAM_HEADER {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t file_size;
    uint64_t mem_size;
    uint64_t align;
};

struct __attribute__((__packed__)) ELF_SECTION_HEADER {
    uint32_t name;
    uint32_t type;
    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    uint32_t link;
    uint32_t info;
    uint64_t addr_align;
    uint64_t entry_size;
};