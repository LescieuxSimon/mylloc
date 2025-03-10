#ifndef NEW_HPP
#define NEW_HPP

#include <alloc/malloc.h>
#include <new>

void* operator new(std::size_t sz);
void* operator new[](std::size_t sz);
void* operator new(std::size_t sz, std::align_val_t al);   // C++17
void* operator new[](std::size_t sz, std::align_val_t al); // C++17

void* operator new(std::size_t sz, const std::nothrow_t tag);
void* operator new[](std::size_t sz, const std::nothrow_t tag);
void* operator new(std::size_t sz, std::align_val_t al,
                   const std::nothrow_t tag); // C++17
void* operator new[](std::size_t sz, std::align_val_t al,
                     const std::nothrow_t tag); // C++17

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, std::align_val_t al) noexcept;   // C++17
void operator delete[](void* ptr, std::align_val_t al) noexcept; // C++17
void operator delete(void* ptr, std::size_t sz) noexcept;        // C++14
void operator delete[](void* ptr, std::size_t sz) noexcept;      // C++14
void operator delete(void* ptr, std::size_t sz,
                     std::align_val_t al) noexcept; // C++17
void operator delete[](void* ptr, std::size_t sz,
                       std::align_val_t al) noexcept; // C++17

void operator delete(void* ptr, std::nothrow_t tag) noexcept;
void operator delete[](void* ptr, std::nothrow_t tag) noexcept;
void operator delete(void* ptr, std::align_val_t al,
                     std::nothrow_t tag) noexcept; // C++17
void operator delete[](void* ptr, std::align_val_t al,
                       std::nothrow_t tag) noexcept; // C++17

#endif // !NEW_HPP
