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

#include"enc2txt.h"

#define INLINE __inline
#define BUFF_SIZE 32768

static char wbuffer[BUFF_SIZE];
static uint bufpos = 0;

static uint bits(uint n);
static void expandLeaf(RULE *rule, CODE code, FILE *output);

static INLINE
uint bits (uint n)
{ uint b = 0;
  while (n)
    { b++; n >>= 1; }
  return b;
}

static INLINE
void expandLeaf(RULE *rule, CODE leaf, FILE *output) {
  if (leaf < CHAR_SIZE) {
    wbuffer[bufpos++] = leaf;
    if (bufpos == BUFF_SIZE) {
      fwrite(wbuffer, 1, BUFF_SIZE, output);
      bufpos = 0;
    }
    return;
  }
  else {
    expandLeaf(rule, rule[leaf].left, output);
    expandLeaf(rule, rule[leaf].right, output); 
    return;
  }
}

void ReadEnd(FILE * input){
  
  uint i;
  RULE *rule;
  uint numRules, txtlen;
  fread(&txtlen, sizeof(uint), 1, input);
  fread(&numRules, sizeof(uint), 1, input);
  printf("txtlen = %d, numRules = %d\n", txtlen, numRules);
  rule = (RULE*)malloc(sizeof(RULE)*numRules);
  for (i = 0; i <= CHAR_SIZE; i++) {
    rule[i].left = (CODE)i;
    rule[i].right = DUMMY_CODE;
  }

  for (i = CHAR_SIZE+1; i < numRules; i++) {
    rule[i].left = DUMMY_CODE;
    rule[i].right = DUMMY_CODE;
  }
}
void DecodeCFG(FILE *input, FILE *output) {
  uint i;
  RULE *rule;
  uint numRules, txtlen;
  BITIN *bitin;
  uint exc, sp;
  uint stack[1024];
  uint newcode, leaf;
  uint bitlen;
  boolean paren;

  fread(&txtlen, sizeof(uint), 1, input);
  fread(&numRules, sizeof(uint), 1, input);
  /* printf("txtlen = %d, numRules = %d\n", txtlen, numRules); */
  rule = (RULE*)malloc(sizeof(RULE)*numRules);
  for (i = 0; i <= CHAR_SIZE; i++) {
    rule[i].left = (CODE)i;
    rule[i].right = DUMMY_CODE;
  }

  for (i = CHAR_SIZE+1; i < numRules; i++) {
    rule[i].left = DUMMY_CODE;
    rule[i].right = DUMMY_CODE;
  }

  /* printf("Decompressing ... "); */
  fflush(stdout);
  bitin = createBitin(input);
  newcode = CHAR_SIZE;
  exc = 0; sp = 0;
  while (1) {
    paren = readBits(bitin, 1);
    if (paren == OP) {
      exc++;
      bitlen = bits(newcode);
      leaf = readBits(bitin, bitlen);
      expandLeaf(rule, leaf, output);
      stack[sp++] = leaf;
    }
    else {
      exc--;
      if (exc == 0) break;
      newcode++;
      rule[newcode].right = stack[--sp];
      rule[newcode].left  = stack[--sp];
      stack[sp++] = newcode;
    }
  }
  fwrite(wbuffer, 1, bufpos, output);
  /* printf("Done!\n"); */
  free(rule);
}
