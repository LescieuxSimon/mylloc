#ifndef SMALL_HPP
#define SMALL_HPP

#include <alloc\utilities.hpp>

#include <bit> // bit_width
#include <mutex>

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
