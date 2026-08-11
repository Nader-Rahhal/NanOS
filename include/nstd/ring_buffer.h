#pragma once

#include "drivers/serial.h"
#include "nstd/managed_array.h"

namespace nstd {
template <typename T, size_t N>
class ring_buffer {
private:
    nstd::managed_array<T, N> data;
    uint32_t head = 0;
public:

    size_t size() const { return N; }

    void clear() {
        data.clear();
    }

    void insert(const T& elem){
        drivers::serial::print("Inserting at position ");
        drivers::serial::print(head);
        drivers::serial::print("\n");
        data[head] = elem;
        head = (head + 1) % N;
    }

    T& operator[](size_t index) { return data[index]; }
};
}