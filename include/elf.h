#pragma once

#include <stdint.h>
#include "util.h"

namespace elf::detail {
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

ELF_HEADER parse_elf_header(uint8_t* elf_buf){
    struct ELF_HEADER hdr;
    util::memcpy(&hdr, elf_buf, sizeof(ELF_HEADER));
    
    if (hdr.magic != 0x464C457F){
        util::serial_puts("exec: bad ELF magic (read_file gave us garbage?)\r\n");
        return hdr;
    }

    return hdr;
}

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
}

namespace elf {
    uint64_t load(uint8_t* elf_buf){
        struct detail::ELF_HEADER hdr = detail::parse_elf_header(elf_buf);

        for (uint16_t i = 0; i < hdr.ph_entry_count; i++){
            struct detail::ELF_PROGRAM_HEADER phdr;
            util::memcpy(&phdr, elf_buf + hdr.ph_offset + i * sizeof(detail::ELF_PROGRAM_HEADER),sizeof(detail::ELF_PROGRAM_HEADER));

            if (phdr.type != 1) continue;

            uint8_t* dest = (uint8_t*)phdr.vaddr;
            util::memcpy(dest, elf_buf + phdr.offset, phdr.file_size);

            // zero the BSS tail (mem_size > file_size for segments with .bss)
            for (uint64_t k = phdr.file_size; k < phdr.mem_size; k++)
                dest[k] = 0;
        }

        return hdr.entry_point;
    }
}
