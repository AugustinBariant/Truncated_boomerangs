// Use AES-based RNG

#ifdef __AES__

#include <smmintrin.h>
#include <wmmintrin.h>
#include <stdint.h>
#include <assert.h>

__m128i key, count;
int next;

void init_aesrand(unsigned long long seed) {
  srand(seed);
  key   = _mm_setr_epi32(rand(), rand(), rand(), rand());
  count = _mm_setr_epi32(rand(), rand(), rand(), rand());
  next = 0;
}

unsigned int aesrand_int32() {
  static __m128i t;
  switch(next++) {
  case 0:
    // Increment counter
    count = _mm_add_epi64(count, _mm_setr_epi32(0,0,1,0));
    
    // 6 AES rounds
    t = count;
    t = _mm_aesenc_si128(t, key);
    t = _mm_aesenc_si128(t, key);
    t = _mm_aesenc_si128(t, key);
    t = _mm_aesenc_si128(t, key);
    t = _mm_aesenc_si128(t, key);
    t = _mm_aesenc_si128(t, key);
    
    return _mm_extract_epi32(t,0);
  case 1:
    return _mm_extract_epi32(t,1);
  case 2:
    return _mm_extract_epi32(t,2);
  case 3:
    next = 0;
    return _mm_extract_epi32(t,3);
  default:
    assert(0);
  }
}

__m128i aesrand_int128() {
  static __m128i t;
  // Increment counter
  count = _mm_add_epi64(count, _mm_setr_epi32(0,0,1,0));
    
  // 6 AES rounds
  t = count;
  t = _mm_aesenc_si128(t, key);
  t = _mm_aesenc_si128(t, key);
  t = _mm_aesenc_si128(t, key);
  t = _mm_aesenc_si128(t, key);
  t = _mm_aesenc_si128(t, key);
  t = _mm_aesenc_si128(t, key);
    
  next = 0;

  return t;
}

struct RNG_state {
  __m128i count;
  __m128i t;
  int next;
};

void* init_aesrand_r(uint32_t seed1, uint32_t seed2) {
  // Internal state: count + t
  struct RNG_state *st = malloc(sizeof(struct RNG_state));
  assert(st);
  st->count = _mm_setr_epi32(seed1, seed2, 0, 0);
  st->next = 0;
  return st;
}

unsigned int aesrand_int32_r(struct RNG_state* st) {
  switch(st->next++) {
  case 0:
    // Increment counter
    st->count = _mm_add_epi64(st->count, _mm_setr_epi32(0,0,1,0));
    
    // 6 AES rounds
    st->t = st->count;
    st->t = _mm_aesenc_si128(st->t, key);
    st->t = _mm_aesenc_si128(st->t, key);
    st->t = _mm_aesenc_si128(st->t, key);
    st->t = _mm_aesenc_si128(st->t, key);
    st->t = _mm_aesenc_si128(st->t, key);
    st->t = _mm_aesenc_si128(st->t, key);
    
    return _mm_extract_epi32(st->t,0);
  case 1:
    return _mm_extract_epi32(st->t,1);
  case 2:
    return _mm_extract_epi32(st->t,2);
  case 3:
    st->next = 0;
    return _mm_extract_epi32(st->t,3);
  default:
    assert(0);
  }
}


__m128i aesrand_int128_r(struct RNG_state* st) {
  // Increment counter
  st->count = _mm_add_epi64(st->count, _mm_setr_epi32(0,0,1,0));
  
  // 6 AES rounds
  st->t = st->count;
  st->t = _mm_aesenc_si128(st->t, key);
  st->t = _mm_aesenc_si128(st->t, key);
  st->t = _mm_aesenc_si128(st->t, key);
  st->t = _mm_aesenc_si128(st->t, key);
  st->t = _mm_aesenc_si128(st->t, key);
  st->t = _mm_aesenc_si128(st->t, key);
  
  st->next=0;
  
  return st->t;
}

#else

#include "util.h"

void init_aesrand(unsigned long long seed) {
  die ("AES-NI support not enabled!\n");
}

unsigned int aesrand_int32(void){
  die ("AES-NI support not enabled!\n");
}

void* init_aesrand_r(uint32_t seed1, uint32_t seed2) {
  die ("AES-NI support not enabled!\n");
}

unsigned int aesrand_int32_r(void* st) {
  die ("AES-NI support not enabled!\n");
}


#endif
