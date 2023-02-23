// Middle rounds of Deoxys Boomerang

#define _XOPEN_SOURCE 600
#define _BSD_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <math.h>

// Getrandom
# if __GLIBC__ > 2 || __GLIBC_MINOR__ > 24

#include <sys/random.h>

#else /* older glibc */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>

int getrandom(void *buf, size_t buflen, unsigned int flags) {
    return syscall(SYS_getrandom, buf, buflen, flags);
}

# endif


// AES rng
#include <smmintrin.h>
#include <wmmintrin.h>

void init_aesrand(unsigned long long seed);
unsigned int aesrand_int32(void);
__m128i aesrand_int128(void);

void* init_aesrand_r(uint32_t seed1, uint32_t seed2);
unsigned int aesrand_int32_r(void* st);
__m128i aesrand_int128_r(void* st);

typedef __m128i state;

#define tabsize(t) (sizeof(t)/sizeof((t)[0]))

// Total samples
#define SAMPLES ((1ULL)<<25)
// Randomize key for chunks of samples
#define CHUNK ((1ULL)<<16)

#define _mm_eq(A,B) _mm_test_all_zeros(A^B,A^B)

// AES SBox

uint8_t SBox[256] =
 {0x63 ,0x7c ,0x77 ,0x7b ,0xf2 ,0x6b ,0x6f ,0xc5 ,0x30 ,0x01 ,0x67 ,0x2b ,0xfe ,0xd7 ,0xab ,0x76
 ,0xca ,0x82 ,0xc9 ,0x7d ,0xfa ,0x59 ,0x47 ,0xf0 ,0xad ,0xd4 ,0xa2 ,0xaf ,0x9c ,0xa4 ,0x72 ,0xc0
 ,0xb7 ,0xfd ,0x93 ,0x26 ,0x36 ,0x3f ,0xf7 ,0xcc ,0x34 ,0xa5 ,0xe5 ,0xf1 ,0x71 ,0xd8 ,0x31 ,0x15
 ,0x04 ,0xc7 ,0x23 ,0xc3 ,0x18 ,0x96 ,0x05 ,0x9a ,0x07 ,0x12 ,0x80 ,0xe2 ,0xeb ,0x27 ,0xb2 ,0x75
 ,0x09 ,0x83 ,0x2c ,0x1a ,0x1b ,0x6e ,0x5a ,0xa0 ,0x52 ,0x3b ,0xd6 ,0xb3 ,0x29 ,0xe3 ,0x2f ,0x84
 ,0x53 ,0xd1 ,0x00 ,0xed ,0x20 ,0xfc ,0xb1 ,0x5b ,0x6a ,0xcb ,0xbe ,0x39 ,0x4a ,0x4c ,0x58 ,0xcf
 ,0xd0 ,0xef ,0xaa ,0xfb ,0x43 ,0x4d ,0x33 ,0x85 ,0x45 ,0xf9 ,0x02 ,0x7f ,0x50 ,0x3c ,0x9f ,0xa8
 ,0x51 ,0xa3 ,0x40 ,0x8f ,0x92 ,0x9d ,0x38 ,0xf5 ,0xbc ,0xb6 ,0xda ,0x21 ,0x10 ,0xff ,0xf3 ,0xd2
 ,0xcd ,0x0c ,0x13 ,0xec ,0x5f ,0x97 ,0x44 ,0x17 ,0xc4 ,0xa7 ,0x7e ,0x3d ,0x64 ,0x5d ,0x19 ,0x73
 ,0x60 ,0x81 ,0x4f ,0xdc ,0x22 ,0x2a ,0x90 ,0x88 ,0x46 ,0xee ,0xb8 ,0x14 ,0xde ,0x5e ,0x0b ,0xdb
 ,0xe0 ,0x32 ,0x3a ,0x0a ,0x49 ,0x06 ,0x24 ,0x5c ,0xc2 ,0xd3 ,0xac ,0x62 ,0x91 ,0x95 ,0xe4 ,0x79
 ,0xe7 ,0xc8 ,0x37 ,0x6d ,0x8d ,0xd5 ,0x4e ,0xa9 ,0x6c ,0x56 ,0xf4 ,0xea ,0x65 ,0x7a ,0xae ,0x08
 ,0xba ,0x78 ,0x25 ,0x2e ,0x1c ,0xa6 ,0xb4 ,0xc6 ,0xe8 ,0xdd ,0x74 ,0x1f ,0x4b ,0xbd ,0x8b ,0x8a
 ,0x70 ,0x3e ,0xb5 ,0x66 ,0x48 ,0x03 ,0xf6 ,0x0e ,0x61 ,0x35 ,0x57 ,0xb9 ,0x86 ,0xc1 ,0x1d ,0x9e
 ,0xe1 ,0xf8 ,0x98 ,0x11 ,0x69 ,0xd9 ,0x8e ,0x94 ,0x9b ,0x1e ,0x87 ,0xe9 ,0xce ,0x55 ,0x28 ,0xdf
 ,0x8c ,0xa1 ,0x89 ,0x0d ,0xbf ,0xe6 ,0x42 ,0x68 ,0x41 ,0x99 ,0x2d ,0x0f ,0xb0 ,0x54 ,0xbb ,0x16};


state AES_encrypt(state X, state K[4], state T, state K_eq[4], state T_eq) {
  /* assert(_mm_eq(K_eq[0], _mm_aesimc_si128(K[0]))); */
  /* assert(_mm_eq(K_eq[1], _mm_aesimc_si128(K[1]))); */
  /* assert(_mm_eq(K_eq[2], _mm_aesimc_si128(K[2]))); */
  /* assert(_mm_eq(K_eq[3], _mm_aesimc_si128(K[3]))); */
  /* assert(_mm_eq(T_eq, _mm_aesimc_si128(T))); */
  
  state Y = X ^ K[0] ^ T;
  Y = _mm_aesenc_si128(Y, K[1]^T);
  Y = _mm_aesenc_si128(Y, K[2]^T);
  Y = _mm_aesenc_si128(Y, K[3]^T);

  return Y;
}

state AES_decrypt(state X, state K[4], state T, state K_eq[4], state T_eq) {
  /* assert(_mm_eq(K_eq[0], _mm_aesimc_si128(K[0]))); */
  /* assert(_mm_eq(K_eq[1], _mm_aesimc_si128(K[1]))); */
  /* assert(_mm_eq(K_eq[2], _mm_aesimc_si128(K[2]))); */
  /* assert(_mm_eq(K_eq[3], _mm_aesimc_si128(K[3]))); */
  /* assert(_mm_eq(T_eq, _mm_aesimc_si128(T))); */
  
  state Y = _mm_aesimc_si128(X) ^ K_eq[3] ^ T_eq;
  Y = _mm_aesdec_si128(Y, K_eq[2]^T_eq);
  Y = _mm_aesdec_si128(Y, K_eq[1]^T_eq);
  Y = _mm_aesdeclast_si128(Y, K[0]^T);

  /* assert(_mm_eq(X, AES_encrypt(Y, K, T, K_eq, T_eq))); */
  return Y;
}

void print_state(__m128i s) {
  uint32_t a =  _mm_extract_epi32(s,0);
  uint32_t b =  _mm_extract_epi32(s,1);
  uint32_t c =  _mm_extract_epi32(s,2);
  uint32_t d =  _mm_extract_epi32(s,3);

  printf ("%02x %02x %02x %02x\n", (a>>(8*0))&0xff, (b>>(8*0))&0xff, (c>>(8*0))&0xff, (d>>(8*0))&0xff);
  printf ("%02x %02x %02x %02x\n", (a>>(8*1))&0xff, (b>>(8*1))&0xff, (c>>(8*1))&0xff, (d>>(8*1))&0xff);
  printf ("%02x %02x %02x %02x\n", (a>>(8*2))&0xff, (b>>(8*2))&0xff, (c>>(8*2))&0xff, (d>>(8*2))&0xff);
  printf ("%02x %02x %02x %02x\n", (a>>(8*3))&0xff, (b>>(8*3))&0xff, (c>>(8*3))&0xff, (d>>(8*3))&0xff);
  printf("\n");
}

int main () {
  if (0) {
    // Test AES rounds
    state X = _mm_setr_epi32(0x03020100,0x07060504,0x0b0a0908,0x0f0e0d0c);
    print_state(X);
    state X0 = _mm_setr_epi32(0,0,0,0);
    state X1 = _mm_setr_epi32(0x100,0,0,1);
    print_state(X0);
    print_state(X1);
    print_state(X0 ^ X1);
    state Z = _mm_set_epi32(0,0,0,0);
    X0 = _mm_aesenc_si128(X0, Z);
    X1 = _mm_aesenc_si128(X1, Z);
    print_state(X0);
    print_state(X1);
    print_state(X0 ^ X1);
    return 0;
  }


  /* Subkey differences */
  state delta_K[4] = {
    //               03020100   07060504   0b0a0908   0f0e0d0c
    _mm_setr_epi32(0x00670000,0x86000000,0x00000000,0x00005f00),
    _mm_setr_epi32(0x00000000,0x00000000,0x00000000,0x00c89bce),
    _mm_setr_epi32(0x7a000000,0x00000000,0x00007500,0x000000fa),
    _mm_setr_epi32(0x00000000,0x00000000,0x00000000,0x00000000),
  };

  state nabla_K[4] = {
    //               03020100   07060504   0b0a0908   0f0e0d0c
    _mm_setr_epi32(0x00000000,0x00000000,0x00000000,0x00000000),
    _mm_setr_epi32(0x8500ac00,0x00000000,0x00000029,0x00000000),
    _mm_setr_epi32(0x000000ed,0x00000000,0x00570000,0xba000000),
    _mm_setr_epi32(0x00000000,0x45ca8f00,0x00000000,0x00000000),
  };

  /* state delta_K_eq[4]; */
  /* state nabla_K_eq[4]; */
  /* for (int i=0; i<4; i++) { */
  /*   delta_K_eq[i] = _mm_aesimc_si128(delta_K[i]); */
  /*   nabla_K_eq[i] = _mm_aesimc_si128(nabla_K[i]); */
  /* } */

 unsigned int seed;
  getrandom(&seed, sizeof(seed), 0);
  init_aesrand(seed);
  srand(seed+1);

  uint64_t count = 0;

  
#pragma omp parallel
  {
    void* rnd_state = init_aesrand_r(rand(), rand());
  
#pragma omp for
    for (uint64_t sample=0; sample<SAMPLES; sample+=CHUNK) {
      // Randomize key
      state K0[4], K0_eq[4];
      state K1[4], K1_eq[4];
      state K2[4], K2_eq[4];
      state K3[4], K3_eq[4];
      for (int i=0; i<4; i++) {
        K0[i] = aesrand_int128();
        K1[i] = K0[i] ^ delta_K[i];
        K2[i] = K0[i] ^ nabla_K[i];
        K3[i] = K0[i] ^ nabla_K[i] ^ delta_K[i];
        K0_eq[i] = _mm_aesimc_si128(K0[i]);
        K1_eq[i] = _mm_aesimc_si128(K1[i]);
        K2_eq[i] = _mm_aesimc_si128(K2[i]);
        K3_eq[i] = _mm_aesimc_si128(K3[i]);
      }

      // Find good transitions

      int a=-1, b=-1, c=-1;
      while (a == -1) {
          int x = aesrand_int32_r(rnd_state) & 0xff;
          int y = x ^ 0xe5;
          if ((SBox[x] ^ SBox[y]) == 0xca)
            a = x;
      }
      while (b == -1) {
          int x = aesrand_int32_r(rnd_state) & 0xff;
          int y = x ^ 0x25;
          if ((SBox[x] ^ SBox[y]) == 0xca)
            b = x;
      }
      
      
      for (uint64_t t=0; t<CHUNK; t++) {
        // Initial state
        state X = aesrand_int128_r(rnd_state);
        // Tweak to randomize the key
        state T = aesrand_int128_r(rnd_state);
        state T_eq = _mm_aesimc_si128(T);

        // Force good transition for intial SBoxes
        state mask = _mm_setr_epi32(0x00ffffff,0xffffffff,0xffffffff,0xff00ffff);
        X = (X & mask) | _mm_setr_epi32(a<<24, 0, 0, b<<16);
        X = _mm_aesenc_si128(X, K0[3]^T);

        if (1) {
          state Z = _mm_setr_epi32(0,0,0,0);
          state U = _mm_aesimc_si128(X) ^ K0_eq[3] ^ T_eq;
          U = _mm_aesdeclast_si128(U, Z);
          state V = _mm_aesimc_si128(X) ^ K2_eq[3] ^ T_eq;
          V = _mm_aesdeclast_si128(V, Z);
          if (!_mm_eq(U ^ _mm_setr_epi32(0xe5000000, 0, 0, 0x250000), V)) {
            print_state(U^V);
            print_state(_mm_setr_epi32(0xe5000000, 0, 0, 0x250000));
          }
          assert(_mm_eq(U ^ _mm_setr_epi32(0xe5000000, 0, 0, 0x250000), V));
        }
        

        // Try boomerang
        state Y0 = AES_decrypt(X, K0, T, K0_eq, T_eq);
        state Y2 = AES_decrypt(X, K2, T, K2_eq, T_eq);
        
        state X1 = AES_encrypt(Y0, K1, T, K1_eq, T_eq);
        state X3 = AES_encrypt(Y2, K3, T, K3_eq, T_eq);

        if (_mm_eq(X1,X3)) {
          __atomic_fetch_add(&count,1,__ATOMIC_RELAXED);
          if (1)
#pragma omp critical
            {
              state U0 = Y0 ^ K0[0] ^ T;
              state U1 = Y0 ^ K1[0] ^ T;
              state U2 = Y2 ^ K2[0] ^ T;
              state U3 = Y2 ^ K3[0] ^ T;
              for (int i=1; i<4; i++) {
                U0 = _mm_aesenc_si128(U0, K0[i]^T);
                U1 = _mm_aesenc_si128(U1, K1[i]^T);
                U2 = _mm_aesenc_si128(U2, K2[i]^T);
                U3 = _mm_aesenc_si128(U3, K3[i]^T);

                printf ("Round %i 0/1:\n", i);
                print_state(U0^U1);
                printf ("Round %i 2/3:\n", i);
                print_state(U2^U3);
                printf ("Round %i 0/2:\n", i);
                print_state(U0^U2);
                printf ("Round %i 1/3:\n", i);
                print_state(U1^U3);
                printf("\n");

                fflush(stdout);
              }
            }
        }
      }
    }
  }

  printf ("Good pairs: %llu\np = 2^%f\n", (long long) count, log2(1.0*count/SAMPLES));
}
