/* 
 *  Copyright (c) 2011 Shirou Maruyama
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above Copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above Copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 */


#include "bits.h"

#define INLINE __inline
//#define DEBUG

#define W_BITS 32
#define BITIN_BUF_LEN 32768 /* BITIN_BUF_LEN*sizeof(uint) bytes */
#define BITOUT_BUF_LEN 32768 /* BITOUT_BUF_LEN*sizeof(uint) bytes */

#ifdef DEBUG
// function for debug
//static
//void printBinary(uint x) {
//  int bit = 1, i;
//  char c[W_BITS];
// 
//  for (i = 0; i < W_BITS; i++) {
//    if (x & bit)
//      c[i] = '1';
//    else
//      c[i] = '0';
//    bit <<= 1;
//  }
//  for (i = W_BITS - 1; i >= 0; i--) {
//      putchar(c[i]);
//  }
//  printf("\n");
//}
#endif

BITOUT *createBitout(FILE *output) {
  BITOUT *b = (BITOUT*)malloc(sizeof(BITOUT));

  b->output = output;
  b->emplen = W_BITS;
  b->bitbuf = 0;
  b->buftop = (uint*)calloc(BITOUT_BUF_LEN+1, sizeof(uint));
  b->bufpos = b->buftop;
  b->bufend = b->buftop + BITOUT_BUF_LEN;
  return b;
}

INLINE
void writeBits(BITOUT *b, uint x, uint wblen) {
  uint s;

#ifdef DEBUG
  if (wblen > W_BITS) {
    fprintf(stderr, "Error: length of write bits (%d) is longer than %d\n", 
	    wblen, W_BITS);
    exit (1);
  }
  if (wblen == 0) {
    return;
  }
#endif

  if (wblen < b->emplen) {
    b->emplen -= wblen;
    b->bitbuf |= x << b->emplen;
  }
  else {
    s = wblen - b->emplen;
    b->bitbuf |= x >> s;
    *(b->bufpos) = b->bitbuf;
    b->bufpos++;
    b->emplen = W_BITS - s;
    if (b->emplen != W_BITS) {
      b->bitbuf = x << b->emplen;
    }
    else {
      b->bitbuf = 0;
    }
 
    if (b->bufpos == b->bufend) {
      fwrite(b->buftop, sizeof(uint), BITOUT_BUF_LEN, b->output);
      memset(b->buftop, 0, sizeof(uint)*BITOUT_BUF_LEN);
      b->bufpos = b->buftop;
    }
  }
}

void flushBitout(BITOUT *b)
{
  uint n;
  if (b->emplen != W_BITS) {
    *(b->bufpos) = b->bitbuf;
    b->bufpos++;
  }
  n = (uint) fwrite(b->buftop, sizeof(uint), b->bufpos - b->buftop, b->output);
  memset(b->buftop, 0, sizeof(uint)*BITOUT_BUF_LEN);
  b->bufpos = b->buftop;
  b->bitbuf = 0;
  b->emplen = W_BITS;
}

BITIN *createBitin(FILE *input) {
  BITIN *b = (BITIN*)malloc(sizeof(BITIN));

  b->input = input;
  b->bitlen = 0;
  b->bitbuf = 0;
  b->buftop = (uint*)calloc(BITIN_BUF_LEN, sizeof(uint));
  b->bufpos = b->bufend = b->buftop;

  return b;
}

INLINE
uint readBits(BITIN *b, uint rblen) {
  uint x;
  uint s, n;

#ifndef DEBUG
  if (rblen > W_BITS) {
    fprintf(stderr, "Error: length of read bits (%d) is longer than %d \n", 
	    rblen, W_BITS);
    exit (1);
  }
  if (rblen == 0) {
    return 0;
  }
#endif

  if (rblen < b->bitlen) {
    x = b->bitbuf >> (W_BITS - rblen);
    b->bitbuf <<= rblen;
    b->bitlen -= rblen;
  }
  else {
    if (b->bufpos == b->bufend) {
      n = (uint) fread(b->buftop, sizeof(uint), BITIN_BUF_LEN, b->input);
      b->bufpos = b->buftop;
      b->bufend = b->buftop + n;
      if (b->bufend < b->buftop) {
	fprintf(stderr, "Error: new bits buffer was not loaded.\n");
	exit(1);
      }
    }

    s = rblen - b->bitlen;
    x = b->bitbuf >> (W_BITS - b->bitlen - s);
    b->bitbuf = *(b->bufpos);
    b->bufpos++;
    b->bitlen = W_BITS - s;
    if (s != 0) {
      x |= b->bitbuf >> b->bitlen;
      b->bitbuf <<= s;
    }
  }

  return x;
}
