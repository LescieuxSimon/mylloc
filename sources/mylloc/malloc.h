#ifndef MALLOC_H
#define MALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

//void* malloc(size_t sz);
//void* calloc(size_t num, size_t sz);
//void* realloc(void* ptr, size_t sz);
//void  free(void* ptr);
//
//void* aligned_alloc(size_t al, size_t sz);                 // C11
//void  free_sized(void* ptr, size_t sz);                    // C23
//void  free_aligned_sized(void* ptr, size_t al, size_t sz); // C23

#ifdef __cplusplus
}
#endif

void* my_malloc(size_t);
void  my_free(void*);

#endif // !MALLOC_HPP
