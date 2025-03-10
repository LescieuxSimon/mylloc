#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <alloc/new.hpp>

// Mockup custom allocator (Replace this with your own!)
struct CustomAllocator {
  static void* allocate(size_t size) {
    return my_malloc(size); // Replace with your allocator
  }

  static void deallocate(void* ptr) {
    my_free(ptr); // Replace with your allocator
  }
};

struct DefaultAllocator {
  static void* allocate(size_t size) {
    return std::malloc(size); // Replace with your allocator
  }

  static void deallocate(void* ptr) {
    std::free(ptr); // Replace with your allocator
  }
};

// Benchmark function
template <typename Allocator>
void benchmark(const char* name, size_t numAllocations, size_t allocationSize) {
  std::vector<void*> pointers(numAllocations);

  constexpr size_t    iterations = 1000;
  double           alloc_time = 0.0;
  double              free_time  = 0.0;

  for (size_t i = 0; i < iterations; i++) {

    auto start = std::chrono::high_resolution_clock::now();

    // Allocate memory
    for (size_t i = 0; i < numAllocations; ++i) {
      pointers[i] = Allocator::allocate(allocationSize);
      if (!pointers[i]) {
        std::cerr << "Allocation failed!\n";
        return;
      }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << name << " alloc took " << duration.count() << " ms\n";
    alloc_time += duration.count();

    for (size_t i = 0; i < numAllocations; ++i) {
      *static_cast<size_t*>(pointers[i]) = i;
    }

    start = std::chrono::high_resolution_clock::now();

    // Free memory
    for (size_t i = 0; i < numAllocations; ++i) {
      Allocator::deallocate(pointers[i]);
    }

    end      = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << name << " free took " << duration.count() << " ms\n";
    free_time += duration.count();
  }

  std::cout << name << " alloc took (average) " << alloc_time / iterations
            << " ms\n";
  std::cout << name << " free took (average) " << free_time / iterations
            << " ms\n";
}

int main() {
  constexpr size_t SMALL_ALLOCS = 1000000; // 1M allocations
  constexpr size_t LARGE_ALLOCS = 10000;   // 10K allocations
  constexpr size_t SMALL_SIZE   = 64;      // 64B allocations
  constexpr size_t LARGE_SIZE   = 8192;    // 8KB allocations

  std::cout << "Benchmarking memory allocation...\n";

  // Default malloc/free benchmark
  /*benchmark<DefaultAllocator>("Default malloc/free (Small)", SMALL_ALLOCS,
                              SMALL_SIZE);*/
  //benchmark<DefaultAllocator>("Default malloc/free (Large)", LARGE_ALLOCS,
  //                                LARGE_SIZE);

  // Custom allocator benchmark
    benchmark<CustomAllocator>("Custom Allocator (Small)", SMALL_ALLOCS,
                               SMALL_SIZE);
    // benchmark<CustomAllocator>("Custom Allocator (Large)", LARGE_ALLOCS,
    //                            LARGE_SIZE);

  return 0;
}
