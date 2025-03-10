#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <Windows.h>
#include <memoryapi.h>
#include <cstddef>
#include <atomic>

// =============================================================================
//
// =============================================================================

void* reserve(size_t size) {
  return VirtualAlloc(nullptr, size, MEM_RESERVE, PAGE_NOACCESS);
}
void* commit(void* where, size_t size) {
  return VirtualAlloc(where, size, MEM_COMMIT, PAGE_READWRITE);
}
void* aquire(size_t size) {
  return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
bool decommit(void* where, size_t size) {
  return VirtualFree(where, size, MEM_DECOMMIT);
}
bool release(void* base) { return VirtualFree(base, 0, MEM_RELEASE); }

// =============================================================================
//
// =============================================================================

union void_ptr {
  static_assert(sizeof(void*) == sizeof(uintptr_t));

  constexpr explicit void_ptr(void* ptr) noexcept : value{ptr} {}
  constexpr void_ptr(uintptr_t val) noexcept : math{val} {}
  constexpr void_ptr(nullptr_t) noexcept : value{nullptr} {}
  constexpr void_ptr(const void_ptr&) noexcept = default;

  constexpr void_ptr& operator=(void* ptr) noexcept {
    value = ptr;
    return *this;
  }
  constexpr void_ptr& operator=(uintptr_t val) noexcept {
    math = val;
    return *this;
  }
  constexpr void_ptr& operator=(const void_ptr&) noexcept = default;

  [[nodiscard]] constexpr void_ptr operator+(uintptr_t val) const noexcept {
    return void_ptr(math + val);
  }
  constexpr void_ptr& operator+=(uintptr_t val) noexcept {
    math += val;
    return *this;
  }

  [[nodiscard]] constexpr void_ptr operator-(uintptr_t val) const noexcept {
    return void_ptr(math - val);
  }
  constexpr void_ptr& operator-=(uintptr_t val) noexcept {
    math -= val;
    return *this;
  }

  [[nodiscard]] constexpr operator uintptr_t() const noexcept { return math; }
  [[nodiscard]] constexpr operator void*() const noexcept { return value; }

  template <typename Type>
  [[nodiscard]] constexpr operator Type*() const noexcept {
    return static_cast<Type*>(value);
  }

private:
  void*     value = nullptr;
  uintptr_t math;
};

// =============================================================================
//
// =============================================================================

#endif // !UTILITIES_HPP
