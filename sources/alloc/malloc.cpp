#include <alloc\malloc.h>
#include <alloc\small.hpp>



// =============================================================================
// =============================================================================
// =============================================================================

void* my_malloc(size_t size) {
  if (size <= 512) { return tmp_small::allocate(size); }
}
void  my_free(void* ptr) { tmp_small::deallocate(ptr); }