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

//post-order encoding of CFGs
//implemented by maruyama

#include "cfg2enc.h"

#define INLINE __inline

static void encodeCFG_rec(CODE code, EDICT *dict, BITOUT *output);
static void putLeaf(uint numcode, CODE lcode, BITOUT *output);
static void putParen(uchar b, BITOUT *output);

static INLINE
uint bits (uint n)
{ uint b = 0;
  while (n)
    { b++; n >>= 1; }
  return b;
}

static INLINE
void putLeaf(uint numcode, uint lvcode, BITOUT *output) {
  uint bitslen = bits(numcode);
  writeBits(output, lvcode, bitslen);
}

static INLINE
void putParen(uchar b, BITOUT *output) {
  if (b == OP) {
    writeBits(output, OP, 1);
  }
  else {
    writeBits(output, CP, 1);
  }
}

static
void encodeCFG_rec(uint code, EDICT *dict, BITOUT *output) {
  if (dict->tcode[code] == DUMMY_CODE) {
    encodeCFG_rec(dict->rule[code].left, dict, output);
    encodeCFG_rec(dict->rule[code].right, dict, output);
    dict->tcode[code] = ++(dict->newcode);
    putParen(CP, output);
  }
  else {
    putParen(OP, output);
    if (code < CHAR_SIZE) {
      putLeaf(dict->newcode, code, output);
    }
    else {
      putLeaf(dict->newcode, dict->tcode[code], output);
    }
  }
}

void EncodeCFG(EDICT *dict, FILE *output) {
  BITOUT *bitout;
  /* printf("Encoding CFG ... "); */
  fflush(stdout);
  dict->newcode = CHAR_SIZE;
  fwrite(&(dict->txtlen), sizeof(uint), 1, output);
  fwrite(&(dict->numRules), sizeof(uint), 1, output);
  bitout = createBitout(output);
  encodeCFG_rec(dict->start, dict, bitout);
  putParen(CP, bitout);
  flushBitout(bitout);
  /* printf("Done!\n"); */
}

EDICT *ReadCFG(FILE *input) {
  uint i;
  uint numRules, txtlen;
  EDICT *dict;
  RULE *rule;
  CODE *tcode;

  fread(&txtlen, sizeof(uint), 1, input);
  fread(&numRules, sizeof(uint), 1, input);
  rule = (RULE *)malloc(sizeof(RULE) * numRules);

  printf("numRules = %d\n", numRules);

  for (i = 0; i <= CHAR_SIZE; i++) {
    rule[i].left = (CODE)i;
    rule[i].right = DUMMY_CODE;
  }

  fread(rule+CHAR_SIZE+1, sizeof(RULE), numRules-(CHAR_SIZE+1), input);

  tcode = (CODE*)malloc(sizeof(CODE)*numRules);
  for (i = 0; i <= CHAR_SIZE; i++) {
    tcode[i] = i;
  }
  for (i = CHAR_SIZE+1; i < numRules; i++) {
    tcode[i] = DUMMY_CODE;
  }

  dict = (EDICT *)malloc(sizeof(EDICT));
  dict->txtlen = txtlen;
  dict->numRules = numRules;
  dict->start = numRules-1;
  dict->rule = rule;
  dict->tcode = tcode;
  dict->newcode = CHAR_SIZE;
  return dict;
}

void DestructEDict(EDICT *dict)
{
  free(dict->rule);
  free(dict->tcode);
  free(dict);
}
