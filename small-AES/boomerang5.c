// Boomerang distinsher on 5-round mini-AES

// 5-round boomerang parameters

// 8-bit Sboxes
// |D^up _in| = 2^32   |D^up _out| = 2^32   p> = 2^-22   p< = 2^-24
// |D^low_in| = 2^32   |D^low_out| = 2^8                 q< = 1
// p_b = 2^-78

// 4-bit Sboxes
// |D^up _in| = 2^16   |D^up _out| = 2^16   p> = 2^-10   q< = 2^-12
// |D^low_in| = 2^16   |D^low_out| = 2^4                 q< = 1
// p_b = 2^-38

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define tabsize(t) (sizeof(t)/sizeof((t)[0]))

/***********************************/
/* T-tables based AES              */
/***********************************/

#define N 4
#define R 5

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
  /* 5-round boomerang               */
  /***********************************/

  printf ("Generating data...\n");
  state delta[1<<N] = {{0}};
  for (unsigned i=0; i<tabsize(delta); i++) {
    delta[i][3] = i;
    MC(delta[i]);
  }
  int n = 0;

  state x = {random(),random(),random(),random()};
  uint64_t batch_size = (1ULL<<(4*N)) * ((1ULL<<N) - 1);
  state *batch = malloc(sizeof(batch[0])*batch_size);

  for (uint64_t i=0; i<1ULL<<(4*N); i++) {
    state y = {x[0], x[1], x[2], x[3]};
    y[0] ^= i & M0;
    y[1] ^= i & M1;
    y[2] ^= i & M2;
    y[3] ^= i & M3;

    encrypt(y, k);
    for (uint64_t j=1; j<1ULL<<N; j++) {
      // Skip j=0
      state z = {y[0], y[1], y[2], y[3]};
      AddKey(z, delta[j]);
      decrypt(z, kInv);
      memcpy(batch[i*((1ULL<<N) - 1)+(j-1)], z, sizeof(z));
    }
  }

  printf ("Sorting data...\n");
  qsort(batch, batch_size, sizeof(batch[0]), cmp_batch);

  for (uint64_t i=1; i<batch_size; i++) {
    if (cmp_batch(batch[i-1], batch[i]) == 0) {
      n++;
    }
  }
  
  printf ("Number of remaining quartets: n = %i\n", n);
  printf ("Expected value: 2 for 5-round AES, 2^-7 for a PRP\n");
}
