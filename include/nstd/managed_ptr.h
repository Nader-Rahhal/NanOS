#pragma once
#include <stddef.h>
#include "new.h"

#include "nstd/utility.h"

namespace nstd {

template <typename T>
class managed_ptr {
public:
    managed_ptr() : data(nullptr), size(0) {}

    managed_ptr(T* ptr, size_t size) : data(ptr), size(size) {}

    ~managed_ptr() { reset(); }

    managed_ptr(managed_ptr&& o) : data(o.data), size(o.size) {
        o.data = nullptr;
        o.size = 0;
    }

    managed_ptr& operator=(managed_ptr&& o) {
        if (this != &o) {
            reset();
            data = o.data;
            size = o.size;
            o.data = nullptr;
            o.size = 0;
        }
        return *this;
    }

    managed_ptr(const managed_ptr&) = delete;
    managed_ptr& operator=(const managed_ptr&) = delete;
    
    managed_ptr& operator=(decltype(nullptr)) {
        reset();
        return *this;
    }

    void reset() {
        if (data) {
            data->~T();
            PhysicalMemoryAllocatorSingleton::getInstance().free(data, size);
            data = nullptr;
            size = 0;
        }
    }

    T* operator->() { return data; }
    T& operator*()  { return *data; }
    T* get()        { return data; }
    explicit operator bool() const { return data != nullptr; }

private:
    T*         data;
    size_t     size;
};

template <typename T, typename Derived = T, typename... Args>
managed_ptr<T> make_managed(Args... args) {
    void* mem = PhysicalMemoryAllocatorSingleton::getInstance().malloc(sizeof(Derived));
    Derived* obj = new (mem) Derived(args...);
    return managed_ptr<T>(obj, sizeof(Derived));
}
}
