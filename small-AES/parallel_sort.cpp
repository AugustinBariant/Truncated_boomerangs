#include <stdint.h>
#include <stdlib.h>
#include <algorithm>
#include <array>

#ifdef CPP17_PARALLEL
#include <execution>
#else
#include <parallel/algorithm>
#endif

extern "C" {
  typedef uint16_t word;
  typedef std::array<word, 4> state;
  typedef struct {
    state c; // Ciphertext C
    state p; // Plaintext \overline P
    word i;  // Plaintext index
    word j;  // Ciphertext index
  } __attribute__((packed)) data;

  void sort_batch (data* batch, uint64_t nb) {
    // Parallel STL sort
#ifdef CPP17_PARALLEL
    // C++17 version
    std::sort(std::execution::par,
#else
    // GNU version
    __gnu_parallel::sort(
#endif
	      batch, batch+nb,
			 [=](const data &a, const data &b){
			   std::array<word, 4> A(a.p), B(b.p);
			   A[0] &= 0x0fff;
			   A[1] &= 0xf0ff;
			   A[2] &= 0xff0f;
			   A[3] &= 0xfff0;
			   B[0] &= 0x0fff;
			   B[1] &= 0xf0ff;
			   B[2] &= 0xff0f;
			   B[3] &= 0xfff0;
			 
			   return (A < B);
			 });
  }
}
