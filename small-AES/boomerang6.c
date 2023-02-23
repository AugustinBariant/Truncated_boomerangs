// Boomerang attack on 6-round mini-AES

// 6-round boomerang parameters with 4-bit Sboxes
// |D^up _in| = 2^16   |D^up _out| = 2^18   p> = 2^-10   q< = 2^-12
// |D^low_in| = 2^18   |D^low_out| = 2^16                q< = 2^-10
// p_b = 2^-60
// p_w = 2^-48*2^-6/2^16 = 2^-70 (extracting only lower key)

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define VERBOSE

#define tabsize(t) (sizeof(t)/sizeof((t)[0]))

/***********************************/
/* T-tables based AES              */
/***********************************/

#define N 4
#define R 6

#if N == 8
typedef uint32_t word ;
#define MASK 0xff
#define M0 0xff000000
#define M1 0xff0000
#define M2 0xff00
#define M3 0xff
#include "tables8.h"
#else
typedef uint16_t word;
#define MASK 0xf
#define M0 0xf000
#define M1 0xf00
#define M2 0xf0
#define M3 0xf
#include "tables4.h"
#endif

typedef word state[4];
typedef struct {
  state c; // Ciphertext C
  state p; // Plaintext \overline P
  word i;  // Plaintext index
  word j;  // Ciphertext index
} __attribute__((packed)) data;

void AddKey(state x, state k) {
  x[0] ^= k[0];
  x[1] ^= k[1];
  x[2] ^= k[2];
  x[3] ^= k[3];
}

void Round(state x, state y) {
    y[0] = Te0[(x[0] >> (3*N)) & MASK] ^
           Te1[(x[1] >> (2*N)) & MASK] ^
           Te2[(x[2] >> (1*N)) & MASK] ^
           Te3[(x[3] >> (0*N)) & MASK];
    y[1] = Te0[(x[1] >> (3*N)) & MASK] ^
           Te1[(x[2] >> (2*N)) & MASK] ^
           Te2[(x[3] >> (1*N)) & MASK] ^
           Te3[(x[0] >> (0*N)) & MASK];
    y[2] = Te0[(x[2] >> (3*N)) & MASK] ^
           Te1[(x[3] >> (2*N)) & MASK] ^
           Te2[(x[0] >> (1*N)) & MASK] ^
           Te3[(x[1] >> (0*N)) & MASK];
    y[3] = Te0[(x[3] >> (3*N)) & MASK] ^
           Te1[(x[0] >> (2*N)) & MASK] ^
           Te2[(x[1] >> (1*N)) & MASK] ^
           Te3[(x[2] >> (0*N)) & MASK];
}

void RoundInv(state x, state y) {
  y[0] = Td0[(x[0] >> (3*N)) & MASK] ^
         Td1[(x[3] >> (2*N)) & MASK] ^
         Td2[(x[2] >> (1*N)) & MASK] ^
         Td3[(x[1] >> (0*N)) & MASK];
  y[1] = Td0[(x[1] >> (3*N)) & MASK] ^
         Td1[(x[0] >> (2*N)) & MASK] ^
         Td2[(x[3] >> (1*N)) & MASK] ^
         Td3[(x[2] >> (0*N)) & MASK];
  y[2] = Td0[(x[2] >> (3*N)) & MASK] ^
         Td1[(x[1] >> (2*N)) & MASK] ^
         Td2[(x[0] >> (1*N)) & MASK] ^
         Td3[(x[3] >> (0*N)) & MASK];
  y[3] = Td0[(x[3] >> (3*N)) & MASK] ^
         Td1[(x[2] >> (2*N)) & MASK] ^
         Td2[(x[1] >> (1*N)) & MASK] ^
         Td3[(x[0] >> (0*N)) & MASK];
}

void SRSB(state x) {
  state y;
  y[0] = (Te4[(x[0] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[1] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[2] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[3] >> (0*N)) & MASK] & M3);
  y[1] = (Te4[(x[1] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[2] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[3] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[0] >> (0*N)) & MASK] & M3);
  y[2] = (Te4[(x[2] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[3] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[0] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[1] >> (0*N)) & MASK] & M3);
  y[3] = (Te4[(x[3] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[0] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[1] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[2] >> (0*N)) & MASK] & M3);
  
  memcpy(x, y, sizeof(state));
}

void SRSBInv(state x) {
  state y;
  y[0] = (Td4[(x[0] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[3] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[2] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[1] >> (0*N)) & MASK] & M3);
  y[1] = (Td4[(x[1] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[0] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[3] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[2] >> (0*N)) & MASK] & M3);
  y[2] = (Td4[(x[2] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[1] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[0] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[3] >> (0*N)) & MASK] & M3);
  y[3] = (Td4[(x[3] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[2] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[1] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[0] >> (0*N)) & MASK] & M3);
  
  memcpy(x, y, sizeof(state));
}

void SB(state x) {
  state y;
  y[0] = (Te4[(x[0] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[0] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[0] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[0] >> (0*N)) & MASK] & M3);
  y[1] = (Te4[(x[1] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[1] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[1] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[1] >> (0*N)) & MASK] & M3);
  y[2] = (Te4[(x[2] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[2] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[2] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[2] >> (0*N)) & MASK] & M3);
  y[3] = (Te4[(x[3] >> (3*N)) & MASK] & M0) ^
         (Te4[(x[3] >> (2*N)) & MASK] & M1) ^
         (Te4[(x[3] >> (1*N)) & MASK] & M2) ^
         (Te4[(x[3] >> (0*N)) & MASK] & M3);
  
  memcpy(x, y, sizeof(state));
}

void SBInv(state x) {
  state y;
  y[0] = (Td4[(x[0] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[0] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[0] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[0] >> (0*N)) & MASK] & M3);
  y[1] = (Td4[(x[1] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[1] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[1] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[1] >> (0*N)) & MASK] & M3);
  y[2] = (Td4[(x[2] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[2] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[2] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[2] >> (0*N)) & MASK] & M3);
  y[3] = (Td4[(x[3] >> (3*N)) & MASK] & M0) ^
         (Td4[(x[3] >> (2*N)) & MASK] & M1) ^
         (Td4[(x[3] >> (1*N)) & MASK] & M2) ^
         (Td4[(x[3] >> (0*N)) & MASK] & M3);
  
  memcpy(x, y, sizeof(state));
}

void MC(state x) {
  state y;
  SRSBInv(x);
  Round(x, y);
  memcpy(x, y, sizeof(state));
}

void MCInv(state x) {
  state y;
  SRSB(x);
  RoundInv(x, y);  
  memcpy(x, y, sizeof(state));
}


void encrypt(state x, state k[R+1]) {
  for (int i=0; i<R; i++) {
    state t;
    AddKey(x,k[i]);
    Round(x, t);
    memcpy(x, t, sizeof(state));
  }

  AddKey(x,k[R]);
}

void decrypt(state x, state k[R+1]) {
  SRSB(x);
  for (int i=R; i>0; i--) {
    state t;
    RoundInv(x, t);
    AddKey(t,k[i]);
    memcpy(x, t, sizeof(state));
  }
  SRSBInv(x);
  AddKey(x,k[0]);
}


void print_state(state x) {
  for (int i=0; i<4; i++)
    printf (N==8? "%08x\n":  "%04x\n", x[i]);
  printf ("\n");
}

void print_quartet(state p1, state p2, state p3, state p4, state k[]) {
  printf ("Candidate Quartet:\n");
  
  state x1, x2, x3, x4;
  memcpy(x1, p1, sizeof(x1));
  memcpy(x2, p2, sizeof(x2));
  memcpy(x3, p3, sizeof(x3));
  memcpy(x4, p4, sizeof(x4));
  for (int i=0; i<=R; i++) {
    for (int j=0; j<4; j++)
      printf (N==8? "%08x | %08x [%08x] || %08x | %08x [%08x] || [%08x] [%08x]\n":
	      "%04x | %04x [%04x] || %04x | %04x [%04x] || [%04x] [%04x]\n",
	      x1[j], x2[j], x1[j]^x2[j], x3[j], x4[j], x3[j]^x4[j], x1[j]^x3[j], x2[j]^x4[j]);
    printf ("\n");

    state t;
    AddKey(x1,k[i]);
    Round(x1, t);
    memcpy(x1, t, sizeof(state));
    AddKey(x2,k[i]);
    Round(x2, t);
    memcpy(x2, t, sizeof(state));
    AddKey(x3,k[i]);
    Round(x3, t);
    memcpy(x3, t, sizeof(state));
    AddKey(x4,k[i]);
    Round(x4, t);
    memcpy(x4, t, sizeof(state));
  }
  printf ("\n");
}

int weight(word x) {
  return
    !!(x & 0xf000) +
    !!(x & 0x0f00) +
    !!(x & 0x00f0) +
    !!(x & 0x000f);
}

word extract_antidiag(state x) {
  return
    (x[0] & 0xf000) |
    (x[1] & 0x000f) |
    (x[2] & 0x00f0) |
    (x[3] & 0x0f00);
}

typedef struct {
  word x[64<<N];
  unsigned n;
} candidates;

word* superbox_inv;

// Pre-compute DDT of a column
// Easier than S-Box by S-Box, but requires morememory
candidates* precompute_candidates() {
  superbox_inv = malloc(sizeof(superbox_inv[0])<<(4*N));
  assert(superbox_inv);
  for (uint64_t i=0; i < 1ULL<<(4*N); i++) {
    state x = {
      i & 0xf000,
      i & 0x000f,
      i & 0x00f0,
      i & 0x0f00
    };
    state y;
    RoundInv(x, y);
    superbox_inv[i] = y[0];
  }
  
  candidates* p = calloc(1ULL<<(4*N), sizeof(p[0]));
  assert(p);
  // Stupid precomputation with complexity 1<<8*N
#pragma omp parallel for
  for (uint64_t delta=1; delta < 1ULL<<(4*N); delta++) {
    // Output difference
    word x = 0;
    // Output value
    do {
      word a = superbox_inv[x];
      word b = superbox_inv[x^delta];
      if (weight(a^b) == 1)
	p[delta].x[p[delta].n++] = x;
      assert(p[delta].n < tabsize(p[delta].x));
    } while (++x);
  }

  return p;
}

int cmp_batch(const void *a, const void *b) {
  state A, B;
  memcpy(&A, a, sizeof(A));
  memcpy(&B, b, sizeof(B));
  A[0] &= ~M0;
  A[1] &= ~M1;
  A[2] &= ~M2;
  A[3] &= ~M3;
  B[0] &= ~M0;
  B[1] &= ~M1;
  B[2] &= ~M2;
  B[3] &= ~M3;
  return memcmp(&A, &B, sizeof(A));
}

int main() {
  srandom(time(NULL));
  state k[R+1];
  state kInv[R+1];
  for (int i=0; i<R+1; i++) {
    for (int j=0; j<4; j++) {
      k[i][j] = random();
      kInv[i][j] = k[i][j];
    }
    if (i)
      MCInv(kInv[i]);
  }

  /***********************************/
  /* Verify AES implementation       */
  /***********************************/
  
  /* state x = {0,0,0,0}; */
  /* print_state(x); */
  /* encrypt(x, k); */
  /* print_state(x); */
  /* decrypt(x, kInv); */
  /* print_state(x); */

  /* state y = {0xdeadbeef,0,0,1}; */
  /* print_state(y); */
  /* encrypt(y, k); */
  /* print_state(y); */
  /* decrypt(y, kInv); */
  /* print_state(y); */


  /***********************************/
  /* 6-round boomerang               */
  /***********************************/

  state x = {random(),random(),random(),random()};
  data *batch = malloc(sizeof(batch[0])<<(8*N));
  if (batch == NULL) {
    fprintf(stderr, "Error: not enough memory (needs 80GB)\n");
    exit(EXIT_FAILURE);
  }
  
  printf ("Generating data...\n");
#pragma omp parallel for
  for (uint64_t i=0; i<1ULL<<(4*N); i++) {
    // Structure active on a diagonal
    state y = {x[0], x[1], x[2], x[3]};
    y[0] ^= i & 0xf000;
    y[1] ^= i & 0x0f00;
    y[2] ^= i & 0x00f0;
    y[3] ^= i & 0x000f;

    encrypt(y, k);
    for (uint64_t j=0; j<1ULL<<(4*N); j++) {
      // Structure active on an anti-diagonal
      state delta = {j & 0xf000, j & 0x000f, j & 0x00f0, j & 0x0f00};
      MC(delta);
      state z = {y[0], y[1], y[2], y[3]};
      AddKey(z, delta);
      decrypt(z, kInv);

      batch[(i<<4*N)+j].i = i;
      batch[(i<<4*N)+j].j = j;
      memcpy(batch[(i<<4*N)+j].c, y, sizeof(y));
      memcpy(batch[(i<<4*N)+j].p, z, sizeof(z));
    }
  }

  printf ("Sorting data...\n");
#if 1
  // Use parallel sort in C++
  void sort_batch (data* batch, uint64_t nb);
  sort_batch(batch, 1ULL<<(8*N));
#else
  // Use qsort
  qsort(batch, 1ULL<<(8*N), sizeof(batch[0]), cmp_batch);
#endif

  printf ("Looking for collisions...\n");
  uint64_t n = 0;
  int counters[1ULL<<(4*N)] = {0};
  candidates* cand = precompute_candidates();
#pragma omp parallel for reduction(+: n)
  for (uint64_t i=1; i<1ULL<<(8*N); i++) {
    if (cmp_batch(batch[i-1].p, batch[i].p) == 0 &&
	(batch[i-1].j || batch[i].j)) { // Filter junk quartets
      n++;
      // Extract key candidates on ciphertext side
      state C0;
      state C1;
      memcpy(&C0, &batch[i-1].c, sizeof(C0));
      memcpy(&C1, &batch[i].c, sizeof(C1));
      MCInv(C0);
      MCInv(C1);
      // Use precomputed table
      for (unsigned j=0; j<cand[batch[i-1].j].n; j++) {
	word kc = cand[batch[i-1].j].x[j] ^ extract_antidiag(C0);

	if (weight(superbox_inv[extract_antidiag(C1)^kc] ^ superbox_inv[extract_antidiag(C1)^batch[i].j^kc]) == 1) {
	  __atomic_fetch_add(&counters[kc],1,__ATOMIC_RELAXED);
#ifdef VERBOSE
	  if (kc == extract_antidiag(kInv[R])) {
#pragma omp critical
	    {
	      state P0, P1;
	      memcpy(&P0, x, sizeof(x));
	      P0[0] ^= batch[i-1].i & 0xf000;
	      P0[1] ^= batch[i-1].i & 0x0f00;
	      P0[2] ^= batch[i-1].i & 0x00f0;
	      P0[3] ^= batch[i-1].i & 0x000f;
	      memcpy(&P1, x, sizeof(x));
	      P1[0] ^= batch[i  ].i & 0xf000;
	      P1[1] ^= batch[i  ].i & 0x0f00;
	      P1[2] ^= batch[i  ].i & 0x00f0;
	      P1[3] ^= batch[i  ].i & 0x000f;

	      print_quartet(P0, batch[i-1].p, P1, batch[i].p, k);
	    }
	  }
#endif
	}
      }
    }
  }

  word max_at = 0;
  int max = 0;
  word kk = 0;
  do {
    if (counters[kk] > max) {
      max = counters[kk];
      max_at = kk;
    }
  } while (++kk);
  
  printf ("Number of remaining quartets: n = %llu\n", (unsigned long long) n);
  printf ("Recovered key: %04x (counter:%i)\n", max_at, max);
  printf ("Actual key: %04x\n", extract_antidiag(kInv[R]));
}
