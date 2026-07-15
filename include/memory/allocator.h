#pragma once
#include <stdint.h>
#include <stddef.h>

#include "pmm.h"
#include "drivers/serial.h"

class PhysicalMemoryAllocator {
public:
    PhysicalMemoryAllocator(MMap* mmap) : pmm(mmap) {};


    void* malloc(size_t size){
        uint64_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        uint64_t addr_out;
        PMM_STATUS status = pmm.alloc_pages(count, addr_out);
        if (status == PMM_STATUS::ALLOC_SUCCESS) {
            return (void*)addr_out;
        } else {
            drivers::serial::print("Failed to allocate memory\n");
            return nullptr;
        }
    }

    void free(void* ptr, size_t size) {
        uint64_t count = (size + PAGE_SIZE - 1) / PAGE_SIZE;
        uint64_t addr = (uint64_t)ptr;
        PMM_STATUS status = pmm.dealloc_pages(addr, count);
        if (status != PMM_STATUS::DEALLOC_SUCCESS) {
            drivers::serial::print("Failed to deallocate memory\n");
        }
    }
private:
    PMM pmm;
};