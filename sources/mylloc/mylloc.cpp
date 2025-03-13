#include <mylloc/mylloc.hpp>

// =============================================================================
// WINDOWS WRAPPER FUNCTIONS
// =============================================================================
#include <memoryapi.h> // For VirtualAlloc and VirtualFree

// Request Windows to reserve an address space in virtual memory. Return a
// pointer to the beginning of the reserved range if the operation was
// succesful, or nullptr otherwise.
_Ret_maybenull_ 
void __cdecl *reserve_memory(_In_opt_ void*  at,
                             _In_     size_t size) {
  return VirtualAlloc(at, size, MEM_RESERVE, PAGE_NOACCESS);
}

// Request Windows to back a previously reserved range of addresses by physical
// memory. Return a pointer to the beginning of the commited range if the
// operation was successful, or nullptr otherwise.
_Ret_maybenull_ _Post_writable_byte_size_(size) 
void __cdecl *commit_memory(_In_ void*  at,
                            _In_ size_t size) {
  return VirtualAlloc(at, size, MEM_COMMIT, PAGE_READWRITE);
}

// Request Windows to provide a physical memory backed address range. Return a
// pointer to the beginning of the commited range if the operation was
// succesful, or nullptr otherwise.
_Ret_maybenull_ _Post_writable_byte_size_(size) 
void __cdecl *reserve_commit_memory(_In_opt_ void*  at,
                                    _In_     size_t size) {
  return VirtualAlloc(at, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

// Request Windows to mark a set of pages as uninteresting without decommiting
// them. Return the input pointer if the operation is successful, or nullptr
// otherwise.
_Ret_maybenull_ 
void __cdecl *reset_memory(_In_ void*  at, 
                           _In_ size_t size) {
  return VirtualAlloc(at, size, MEM_RESET, PAGE_NOACCESS);
}

// Request Windows to restore pages previously marked as 'reset'. Return a
// pointer to the beginning of the requested range if the operation is
// successsful, or nullptr otherwise.
_Ret_maybenull_ _Post_writable_byte_size_(size)
void __cdecl *reset_undo_memory(_In_ void*  at, 
                                _In_ size_t size) {
  return VirtualAlloc(at, size, MEM_RESET_UNDO, PAGE_READWRITE);
}

// Request Windows to liberate the physical memory associated with a range of
// virtual addresses. Return true if the operation is successful, or false
// otherwise.
_Success_(return != false) 
bool __cdecl decommit_memory(_In_ void*  at,
                             _In_ size_t size) {
  return VirtualFree(at, size, MEM_DECOMMIT);
}

// Request Windows to free a reserved range of virtual addresses. Return true if
// the operation was successful, or false otherwise.
_Success_(return != false) 
bool __cdecl release_memory(_In_ void* at) {
  return VirtualFree(at, 0, MEM_RELEASE);
}

// =============================================================================
// VOID POINTER ARITHMETICS
// =============================================================================

//
union void_ptr {
  // Constructors.
  constexpr explicit void_ptr(void *ptr) noexcept : pointer{ptr} {}
  constexpr explicit void_ptr(nullptr_t) noexcept : pointer{nullptr} {}
  constexpr void_ptr(uintptr_t val) noexcept : value{val} {}
  constexpr void_ptr(const void_ptr &) noexcept = default;

  // Assignment operators.
  constexpr void_ptr &operator=(const void_ptr &) noexcept = default;
  constexpr void_ptr &operator=(void *ptr) noexcept {
    return operator=(void_ptr(ptr));
  }
  constexpr void_ptr &operator=(uintptr_t val) noexcept {
    return operator=(void_ptr(val));
  }

  // Addition operators.
  [[nodiscard]] constexpr void_ptr operator+(uintptr_t val) const noexcept {
    return void_ptr(value + val);
  }
  [[nodiscard]] constexpr void_ptr operator+(void_ptr other) const noexcept {
    return void_ptr(value + other.value);
  }

  // Addition assignment operators.
  constexpr void_ptr &operator+=(uintptr_t val) noexcept {
    value + val;
    return *this;
  }
  constexpr void_ptr &operator+=(void_ptr other) noexcept {
    value += other.value;
    return *this;
  }

  // Substraction operators.
  [[nodiscard]] constexpr void_ptr operator-(uintptr_t val) const noexcept {
    return void_ptr(value - val);
  }
  [[nodiscard]] constexpr void_ptr operator-(void_ptr other) const noexcept {
    return void_ptr(value - other.value);
  }

  // Substraction assignment operators.
  constexpr void_ptr &operator-=(uintptr_t val) noexcept {
    value -= val;
    return *this;
  }
  constexpr void_ptr &operator-=(void_ptr other) noexcept {
    value -= other.value;
    return *this;
  }

  // Implicit convertion operators.
  [[nodiscard]] constexpr operator uintptr_t() const noexcept { return value; }
  [[nodiscard]] constexpr operator void *() const noexcept { return pointer; }

private:
  // Make sure this wrapper does not inclure any memory overhead.
  static_assert(sizeof(void *) == sizeof(uintptr_t));

  void *pointer = nullptr; // Pointer representation of the value.
  uintptr_t value;         // Integer representation of the value.
};

// =============================================================================
// C MEMORY MANAGEMENT FUNCTIONS
// =============================================================================

void *malloc(size_t sz);
void *calloc(size_t num, size_t sz);
void *realloc(void *ptr, size_t sz);
void free(void *ptr);

void *aligned_alloc(size_t al, size_t sz);                // C11
void free_sized(void *ptr, size_t sz);                    // C23
void free_aligned_sized(void *ptr, size_t al, size_t sz); // C23

// =============================================================================
// C++ MEMORY MANAGEMENT FUNCTIONS
// =============================================================================

void *operator new(std::size_t sz);
void *operator new[](std::size_t sz);
void *operator new(std::size_t sz, std::align_val_t al);   // C++17
void *operator new[](std::size_t sz, std::align_val_t al); // C++17

void *operator new(std::size_t sz, const std::nothrow_t tag);
void *operator new[](std::size_t sz, const std::nothrow_t tag);
void *operator new(std::size_t sz, std::align_val_t al,
                   const std::nothrow_t tag); // C++17
void *operator new[](std::size_t sz, std::align_val_t al,
                     const std::nothrow_t tag); // C++17

void operator delete(void *ptr) noexcept;
void operator delete[](void *ptr) noexcept;
void operator delete(void *ptr, std::align_val_t al) noexcept;   // C++17
void operator delete[](void *ptr, std::align_val_t al) noexcept; // C++17
void operator delete(void *ptr, std::size_t sz) noexcept;        // C++14
void operator delete[](void *ptr, std::size_t sz) noexcept;      // C++14
void operator delete(void *ptr, std::size_t sz,
                     std::align_val_t al) noexcept; // C++17
void operator delete[](void *ptr, std::size_t sz,
                       std::align_val_t al) noexcept; // C++17

void operator delete(void *ptr, std::nothrow_t tag) noexcept;
void operator delete[](void *ptr, std::nothrow_t tag) noexcept;
void operator delete(void *ptr, std::align_val_t al,
                     std::nothrow_t tag) noexcept; // C++17
void operator delete[](void *ptr, std::align_val_t al,
                       std::nothrow_t tag) noexcept; // C++17

// =============================================================================
// =============================================================================
// =============================================================================

struct heap_bin {
  static constexpr size_t size = 0x10000;

  void_ptr memory = nullptr;
  size_t   next_free;
  size_t   type;
  size_t   used;
};

class heap_bin_manager {
  static constexpr size_t invalid = ~size_t{0};

public:
  static constexpr size_t page_count = 256;
  static constexpr size_t page_size  = heap_bin::size / sizeof(heap_bin);

public:
  heap_bin_manager() : reserved{reserve(0x1000000000)} {}
  ~heap_bin_manager() { release(reserved); }

  heap_bin* get_new_bin() {
    std::lock_guard<std::mutex> guard(mutex);
    if (free_index == invalid) {
      const size_t page = in_use / page_size;
      const size_t pos  = in_use % page_size;

      if (bins[page] == nullptr) {
        bins[page] = static_cast<heap_bin*>(aquire(heap_bin::size));
        if (bins[page] == nullptr) { return nullptr; }
      }

      heap_bin* bin = &bins[page][pos];
      bin->memory   = reserved + in_use * heap_bin::size;
      void* ptr     = commit(bin->memory, heap_bin::size);
      if (ptr == nullptr || ptr != bin->memory) { return nullptr; }

      in_use         += 1;
      bin->next_free  = invalid;
      return bin;

    } else {
      const size_t page = free_index / page_size;
      const size_t pos  = free_index % page_size;

      heap_bin* bin = &bins[page][pos];
      void*     ptr = undo_reset_memory(bin->memory, heap_bin::size);
      if (ptr == nullptr || ptr != bin->memory) { return nullptr; }

      free_index     = bin->next_free;
      bin->next_free = invalid;
      return bin;
    }
  }
  bool return_bin(heap_bin* bin) {
    std::lock_guard<std::mutex> guard(mutex);
    if (!reset_memory(bin->memory, heap_bin::size)) { return false; }
    bin->next_free = free_index;
    free_index     = (void_ptr(bin->memory) - reserved) / heap_bin::size;
    return true;
  }
  heap_bin* get_bin_for(void* ptr) {
    const size_t index = (void_ptr(ptr) - reserved) / heap_bin::size;
    const size_t page  = index / page_size;
    const size_t pos   = index % page_size;
    return bins[page] ? &bins[page][pos] : nullptr;
  }

private:
  void_ptr reserved = nullptr;

  heap_bin* bins[256]  = {nullptr};
  size_t    free_index = invalid;
  size_t    in_use     = 0;

  std::mutex mutex;
} bin_mgr;

// =============================================================================
// =============================================================================
// =============================================================================

namespace tmp_small {

struct heap_block {
  heap_block* next = nullptr;
  heap_block* prev = nullptr;
};

struct heap_block_list {
  // Keep the same structure as the heap_block to enable safe pointer casting
  // between the two structures.
  heap_block* next = nullptr;
  heap_block* prev = nullptr; // Since this should always be used as the head of
                              // the free list, the previous pointer would
                              // technically always be null. Maybe use this
                              // field to store some other information instead?

  heap_bin* last_bin        = nullptr;
  size_t    block_formatted = 0;

  // Any operation on a block free list should aquire the mutex first.
  std::mutex mutex;
};

struct heap_allocator {
  static constexpr size_t min_block_type = 4; // (2^4) 16 bytes bin.
  static constexpr size_t max_block_type = 9; // (2^9) 512 bytes bin.
  static constexpr size_t block_types    = max_block_type - min_block_type + 1;

  heap_block_list free_list[block_types];
} global_allocator;

void format_bin(heap_bin* bin) {
  const size_t block_size = 1ull
                            << (bin->type + heap_allocator::min_block_type);
  const size_t block_count = heap_bin::size / block_size;

  heap_block* prev = nullptr;
  heap_block* curr = bin->memory;
  heap_block* next = bin->memory + block_size;
  for (size_t i = 0; i < block_count; i++) {
    curr->prev = prev;
    curr->next = next;

    prev = curr;
    curr = next;
    next = void_ptr(next) + block_size;
  }
  prev->next = nullptr;
}

void clean_bin(heap_bin* bin) {
  const size_t block_size = 1ull
                            << (bin->type + heap_allocator::min_block_type);
  const size_t block_count = heap_bin::size / block_size;

  heap_block* block = bin->memory;
  for (size_t i = 0; i < block_count; i++) {
    if (block->next) { block->next->prev = block->prev; }
    if (block->prev) { block->prev->next = block->next; }
    block = void_ptr(block) + block_size;
  }
}

void* allocate(size_t size) {
  const size_t block_type =
      std::bit_width(size - 1) - heap_allocator::min_block_type;
  heap_block* block = nullptr;

  heap_block_list& free_list = global_allocator.free_list[block_type];
  std::lock_guard<std::mutex> guard(free_list.mutex);

  if (free_list.next == nullptr) {
    const size_t block_size = 1ull
                              << (block_type + heap_allocator::min_block_type);
    const size_t block_count = heap_bin::size / block_size;
    if (free_list.last_bin != nullptr && free_list.block_formatted < block_count) {
      heap_bin* bin = free_list.last_bin;
      block         = (bin->memory + free_list.block_formatted * block_size);
      free_list.block_formatted += 1;
      bin->used                 += 1;
    } else {
      heap_bin* bin = bin_mgr.get_new_bin();
      bin->type     = block_type;
      bin->used     += 1;

      block                     = bin->memory;
      free_list.last_bin        = bin;
      free_list.block_formatted = 1;
    }

  } else {
    // Use the next available block as our return value.
    block          = free_list.next;
    free_list.next = block->next;
    if (block->next) {
      // The new 'next' block should point back to the head.
      block->next->prev = reinterpret_cast<heap_block*>(&free_list);
    }

    // Find the bin of this block and increment its use count.
    bin_mgr.get_bin_for(block)->used += 1;
  }

  return block;
}

void deallocate(void* ptr) {
  if (heap_bin* bin = bin_mgr.get_bin_for(ptr)) {
    heap_block* block = static_cast<heap_block*>(ptr);

    heap_block_list& free_list = global_allocator.free_list[bin->type];
    std::lock_guard<std::mutex> guard(free_list.mutex);

    block->next = free_list.next;
    block->prev = reinterpret_cast<heap_block*>(&free_list);
    if (free_list.next) { free_list.next->prev = block; }
    free_list.next = block;

    if (--bin->used == 0) {
      if (free_list.last_bin == bin) {
        free_list.last_bin        = nullptr;
        free_list.block_formatted = 0;
      }
      clean_bin(bin);
      bin_mgr.return_bin(bin);
    }
  }
}

} // namespace tmp_small

#endif // !SMALL_HPP
