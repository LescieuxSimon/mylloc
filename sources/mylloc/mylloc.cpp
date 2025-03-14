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
  constexpr void_ptr(nullptr_t) noexcept : pointer{nullptr} {}
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

  // Explicit convertion operator.
  template <typename Type>
  [[nodiscard]] constexpr operator Type *() const noexcept {
    return static_cast<Type *>(pointer);
  }

private:
  // Make sure this wrapper does not inclure any memory overhead.
  static_assert(sizeof(void *) == sizeof(uintptr_t));

  void *pointer = nullptr; // Pointer representation of the value.
  uintptr_t value;         // Integer representation of the value.
};

// =============================================================================
// HEAP BINS
// =============================================================================
#include <mutex> // For std::mutex.

//
struct heap_bin {
    //
  static constexpr size_t size = 0x10000; // 64Kb.

  void_ptr memory = nullptr; //
  size_t next_free;          //
  size_t type;               //
  size_t used;               //
};

struct heap_bin_list__ {
  static constexpr size_t invalid = ~size_t{0};
  static constexpr size_t max_memory = 0x1000000000; // 64Gb.
  static constexpr size_t page_byte_size = heap_bin::size;
  static constexpr size_t page_size = page_byte_size / sizeof(heap_bin);
  static constexpr size_t page_count = max_memory / page_size / heap_bin::size;

  heap_bin_list__() : reserved{reserve_memory(nullptr, max_memory)} {}
  ~heap_bin_list__() { release_memory(reserved); }

  void_ptr reserved = nullptr;            //
  heap_bin *bins[page_count] = {nullptr}; //
  size_t free_index = invalid;            //
  size_t used_bins = 0;                   //

  std::mutex mutex; //
} heap_bin_list;

//
heap_bin *new_bin() {
  auto &list = heap_bin_list;
  std::lock_guard<std::mutex> guard(list.mutex); //

  // If a free index is available, use it. Otherwise, instantiate a new bin.
  if (list.free_index != list.invalid) {
    // Get the free bin.
    const size_t page = list.free_index / list.page_size;
    const size_t pos = list.free_index % list.page_size;
    heap_bin *bin = &list.bins[page][pos];

    // Reactivate the bin's memory.
    if (nullptr == reset_undo_memory(bin->memory, heap_bin::size)) {
      return nullptr;
    }

    list.free_index = bin->next_free;
    return bin;

  } else {
    // Get the theoretical next bin.
    const size_t page = list.used_bins / list.page_size;
    const size_t pos = list.used_bins % list.page_size;

    if (list.bins[page] == nullptr) {
      // We've ran out of allocated bin pages.
      void *new_page = reserve_commit_memory(nullptr, list.page_byte_size);
      if (new_page == nullptr) {
        return nullptr;
      }
      list.bins[page] = static_cast<heap_bin *>(new_page);
    }
    heap_bin *bin = &list.bins[page][pos];

    // Activate the bin's memory.
    void *at = list.reserved + list.used_bins * heap_bin::size;
    if (nullptr == commit_memory(at, heap_bin::size)) {
      return nullptr;
    }

    list.used_bins++;
    return bin;
  }
}

//
bool return_bin(heap_bin *bin) {
  auto &list = heap_bin_list;

  // Deactivate the bin's memory.
  if (!reset_memory(bin->memory, heap_bin::size)) {
    return false;
  }

  std::lock_guard<std::mutex> guard(list.mutex); //

  bin->next_free = list.free_index;
  // Abuse the integer truncation to round down to the nearest bin.
  list.free_index = (void_ptr(bin->memory) - list.reserved) / heap_bin::size;
  return true;
}

//
heap_bin *get_bin_for(void *ptr) {
  auto &list = heap_bin_list;

  // Abuse the integer truncation to round down to the nearest bin.
  const size_t index = (void_ptr(ptr) - list.reserved) / heap_bin::size;
  const size_t page = index / list.page_size;
  const size_t pos = index % list.page_size;

  return page < list.page_count && list.bins[page] ? &list.bins[page][pos]
                                                   : nullptr;
}

// =============================================================================
// SMALL HEAP BLOCKS
// =============================================================================

struct small_heap_block {
  small_heap_block *next;
  small_heap_block *prev;
};

struct small_heap_block_list {
  // Keep the same structure as the heap_block to enable safe pointer casting
  // between the two structures.
  small_heap_block *next = nullptr;
  small_heap_block *prev =
      nullptr; // Since this should always be used as the head of
               // the free list, the previous pointer would
               // technically always be null. Maybe use this
               // field to store some other information instead?

  heap_bin *last_bin = nullptr;
  size_t block_formatted = 0;

  // Any operation on a block free list should aquire the mutex first.
  std::mutex mutex;
};

// =============================================================================
// SMALL HEAP ALLOCATIONS
// =============================================================================
#include <bit>   // for std::bit_width
#include <mutex> // For std::mutex.

struct  {
  static constexpr size_t min_type = 4; // (2^4) 16 bytes bin.
  static constexpr size_t max_type = 9; // (2^9) 512 bytes bin.
  static constexpr size_t types = max_type - min_type + 1;

  small_heap_block_list free_list[types];
} small_block_allocator;

void *allocate_small_block(size_t size) {
  auto &alloc = small_block_allocator;

  const size_t type = std::bit_width(size - 1) - alloc.min_type;
  small_heap_block_list &list = alloc.free_list[type];
  std::lock_guard<std::mutex> guard(list.mutex);

  small_heap_block *block = nullptr;
  if (list.next != nullptr) {
    block = list.next;
    list.next = block->next;

    if (list.next) {
      // The new 'next' block should point back to the head.
      list.next->prev = reinterpret_cast<small_heap_block *>(&list);
    }

    // Find the bin of this block and increment its use count.
    get_bin_for(block)->used += 1;

  } else {
    const size_t block_size = 1ull << (type + alloc.min_type);
    const size_t block_count = heap_bin::size / block_size;

    if (list.last_bin != nullptr && list.block_formatted < block_count) {
      heap_bin *bin = list.last_bin;

      block = static_cast<small_heap_block*>(bin->memory + list.block_formatted * block_size);
      list.block_formatted += 1;
      bin->used += 1;

    } else {
      heap_bin *bin = new_bin();
      bin->type = type;
      bin->used += 1;

      block = bin->memory;
      list.last_bin = bin;
      list.block_formatted = 1;
    }
  }

  return block;
}

void deallocate_small_block(void *ptr) {
  auto &alloc = small_block_allocator;

  if (heap_bin *bin = get_bin_for(ptr)) {
    small_heap_block *block = static_cast<small_heap_block *>(ptr);

    small_heap_block_list &list = alloc.free_list[bin->type];
    std::lock_guard<std::mutex> guard(list.mutex);

    block->next = list.next;
    block->prev = reinterpret_cast<small_heap_block *>(&list);

    if (list.next) { list.next->prev = block; }
    list.next = block;

    if (--bin->used == 0) {
      if (list.last_bin == bin) {
        list.last_bin = nullptr;
        list.block_formatted = 0;
      }

      const size_t block_size = 1ull << (bin->type + alloc.min_type);
      const size_t block_count = heap_bin::size / block_size;

      small_heap_block *current = bin->memory;
      for (size_t i = 0; i < block_count; i++) {
        if (current->next) { current->next->prev = current->prev; }
        if (current->prev) { current->prev->next = current->next; }
        current = void_ptr(current) + block_size;
      }

      return_bin(bin);
    }
  }
}

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
