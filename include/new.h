#pragma once
// Freestanding replacement for <new>: just the placement-new operators.
// No libstdc++ is available in the kernel, so declare these ourselves.
// __SIZE_TYPE__ is a compiler builtin, so no <stddef.h> is required.

inline void* operator new(__SIZE_TYPE__, void* ptr) noexcept { return ptr; }
inline void* operator new[](__SIZE_TYPE__, void* ptr) noexcept { return ptr; }

inline void operator delete(void*, void*) noexcept {}
inline void operator delete[](void*, void*) noexcept {}

// Non-placement deallocation. These are never meant to actually free memory
// (the kernel uses its own allocator); they only exist to satisfy references
// emitted by virtual destructors' deleting-destructor vtable slots. No kernel
// code deletes through these, so the empty bodies are safe.
inline void operator delete(void*) noexcept {}
inline void operator delete[](void*) noexcept {}
inline void operator delete(void*, __SIZE_TYPE__) noexcept {}
inline void operator delete[](void*, __SIZE_TYPE__) noexcept {}
