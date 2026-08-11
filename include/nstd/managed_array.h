#pragma once

#include "memory/allocator.h"

namespace nstd {
template <typename T, size_t N>
class managed_array {
public:
    managed_array() : data(nullptr) {
        data = (T*)PhysicalMemoryAllocatorSingleton::getInstance().getInstance().malloc(N * sizeof(T));
        for (size_t i = 0; i < N; i++) new (&data[i]) T();
    }

    ~managed_array(){ reset(); }

    managed_array(const managed_array&) = delete;
    managed_array& operator=(const managed_array&) = delete;

    managed_array(managed_array&& o) : data(o.data) { o.data = nullptr; }

    managed_array& operator=(managed_array&& o) {
        if (this != &o) {
            reset();
            data = o.data;
            o.data = nullptr;
        }
        return *this;
    }

    T& operator[](size_t index) { return data[index]; }
    T* get() { return data; }
    size_t size() const { return N; }

    void clear() {
        for (size_t i = 0; i < N; i++) data[i] = T();
    }

private:
    void reset() {
        if (data) {
            for (size_t i = N; i-- > 0; ) data[i].~T();
            PhysicalMemoryAllocatorSingleton::getInstance().getInstance().free(data, N * sizeof(T));
            data = nullptr;
        }
    }

    T* data;
};
}