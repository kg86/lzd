#include <string>
#include <vector>
#include <iostream>

#include "slp2enc.hpp"

#include "lcacomp/cfg2enc.h"
#include "lcacomp/enc2txt.h"

#define INLINE __inline
#define BUFF_SIZE 32768

static char wbuffer[BUFF_SIZE];
static uint bufpos = 0;

static uint bits(uint n);
static void expandLeaf(RULE *rule, CODE code);

static INLINE
unsigned int bits (unsigned int n)
{ unsigned int b = 0;
  while (n)
    { b++; n >>= 1; }
  return b;
}

static INLINE
void expandLeaf(RULE *rule, CODE leaf) {
  if (leaf < CHAR_SIZE) {
    wbuffer[bufpos++] = leaf;
    if (bufpos == BUFF_SIZE) {
      // fwrite(wbuffer, 1, BUFF_SIZE, output);
      bufpos = 0;
    }
    return;
  }
  else {
    // expandLeaf(rule, rule[leaf].left, output);
    // expandLeaf(rule, rule[leaf].right, output); 
    expandLeaf(rule, rule[leaf].left);
    expandLeaf(rule, rule[leaf].right); 
    return;
  }
}

void slp2enc(std::vector<std::pair<unsigned int, unsigned int> > & vars, unsigned int decompressedSize, const std::string & outputFilename){
  EDICT *edict = (EDICT*)malloc(sizeof(EDICT));
  std::string t;
  unsigned int i;
  edict->txtlen = decompressedSize;
  edict->start = (unsigned int) vars.size();
  edict->numRules = (unsigned int) vars.size() + 1;
  edict->start = edict->numRules-1;
  edict->rule = (RULE*)malloc(edict->numRules*sizeof(RULE));
  edict->tcode = (CODE*)malloc(edict->numRules*sizeof(CODE));
  edict->newcode = CHAR_SIZE;
  // std::cout << "numrules=" << edict->numRules << std::endl;
  for (i = 0; i <= CHAR_SIZE; i++) {
    edict->rule[i].left = (CODE)i;
    edict->rule[i].right = DUMMY_CODE;
  }

  for (i = CHAR_SIZE; i < vars.size(); i++) {
    CODE left, right;
    if (vars[i].first < CHAR_SIZE) left = (CODE) vars[i].first;
    else left = (CODE) (vars[i].first + 1);
    if (vars[i].second < CHAR_SIZE) right = (CODE) vars[i].second;
    else right = (CODE) (vars[i].second + 1);
    edict->rule[i+1].left = left;
    edict->rule[i+1].right = right;
  }

  for(i = 0; i <= CHAR_SIZE; i++){
    edict->tcode[i] = i;
  }
  for(i = CHAR_SIZE + 1; i < edict->numRules; i++){
    edict->tcode[i] = DUMMY_CODE;
  }
  FILE *output;
  output = fopen(outputFilename.c_str(), "wb");
  if (output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }
  EncodeCFG(edict, output);
  DestructEDict(edict);
  fclose(output);
}

void enc2slp(FILE *input, std::vector<std::pair<int, int> > & vars) {
  uint i;
  RULE *rule;
  uint numRules, txtlen;
  BITIN *bitin;
  uint exc, sp;
  uint stack[1024];
  uint newcode, leaf;
  uint bitlen;
  boolean paren;
  // FILE * output=NULL;

  fread(&txtlen, sizeof(uint), 1, input);
  fread(&numRules, sizeof(uint), 1, input);
  // printf("txtlen = %d, numRules = %d\n", txtlen, numRules);
  vars.resize(numRules-1);
  rule = (RULE*)malloc(sizeof(RULE)*numRules);
  for (i = 0; i <= CHAR_SIZE; i++) {
    rule[i].left = (CODE)i;
    rule[i].right = DUMMY_CODE;
    vars[i].first = -1;
    vars[i].second = -1;
  }

  for (i = CHAR_SIZE+1; i < numRules; i++) {
    rule[i].left = DUMMY_CODE;
    rule[i].right = DUMMY_CODE;
  }

  // printf("Decompressing ... ");
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
      expandLeaf(rule, leaf);
      stack[sp++] = leaf;
    }
    else {
      exc--;
      if (exc == 0) break;
      newcode++;
      CODE left, right;
      right = stack[--sp];
      left = stack[--sp];
      // rule[newcode].right = stack[--sp];
      // rule[newcode].left  = stack[--sp];
      if (newcode == CHAR_SIZE){}
      else {
        if (right > CHAR_SIZE) right--;
        if (left > CHAR_SIZE) left--;
        vars[newcode-1].second = (int)right;
        vars[newcode-1].second = (int)left;
      }
      stack[sp++] = newcode;
    }
  }
  // fwrite(wbuffer, 1, bufpos, output);
  // printf("Done!\n");
  free(rule);
}

int encSLP_decompress(std::string & in_fname, std::string & out_fname) {
  FILE *input, *output;

  input = fopen(in_fname.c_str(), "rb");
  output = fopen(out_fname.c_str(), "w");
  if (input == NULL || output == NULL) {
    printf("File open error.\n");
    return 0;
  }
  DecodeCFG(input, output);
  fclose(input); fclose(output);
  return 1;
}
