#include <alloc\malloc.h>
#include <alloc\small.hpp>



// =============================================================================
// =============================================================================
// =============================================================================

void* my_malloc(size_t size) {
  if (size <= 512) { return tmp_small::allocate(size); }
}
void my_free(void *ptr) {
  if (heap_bin *bin = bin_mgr.get_bin_for(ptr)) {
    if (bin->type <= tmp_small::heap_allocator::max_block_type) {
      // Falls within the small block types.
      tmp_small::deallocate(ptr);
    } else {
      // Must be a normal allocation.
    }
  } else {
    // If no bin exist for this block, it must be a large block.
  }
}