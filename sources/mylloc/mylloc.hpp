#ifndef NEW_HPP
#define NEW_HPP

#include <new>

#ifdef __cplusplus
extern "C" {
#endif

// void* malloc(size_t sz);
// void* calloc(size_t num, size_t sz);
// void* realloc(void* ptr, size_t sz);
// void  free(void* ptr);
//
// void* aligned_alloc(size_t al, size_t sz);                 // C11
// void  free_sized(void* ptr, size_t sz);                    // C23
// void  free_aligned_sized(void* ptr, size_t al, size_t sz); // C23

#ifdef __cplusplus
}
#endif

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
