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

#include "txt2cfg_offline.h"

#define INLINE __inline

static DICT *createDictionary  ();
static CODE addRule2Dictionary (DICT *dict, CODE left, CODE right);
static CODE searchRule         (DICT *dict, CODE left, CODE right);
static CODE *preReading        (FILE *input, uint size_w);
static boolean isMinimal          (CODE *w, int i);
static boolean isMaximal          (CODE *w, int i);
static boolean isRepetition       (CODE *w, int i);
static boolean isPair             (CODE *w, int i);
static uint computeLCAd        (CODE i, CODE j);


static INLINE
boolean isRepetition(CODE *w, int src)
{	
  if (w[src] != w[src+1]) {
    return false;
  }
  return true;
}

static INLINE
boolean isMinimal(CODE *w, int i)
{
  if ((w[i-1] > w[i]) && (w[i] < w[i+1])) {
    return true;
  } else {
    return false;
  }
}

static INLINE
uint computeLCAd(CODE i, CODE j)
{
  uint Ni, Nj;
  uint x;
  Ni = 2*i - 1;
  Nj = 2*j - 1;
  x = Ni ^ Nj;
  x = (uint)floor(LOG2(x));
  return x;
}

static INLINE
boolean isMaximal(CODE *w, int i)
{
  if (!(w[i-1] < w[i] && w[i] < w[i+1] && w[i+1] < w[i+2]) &&
      !(w[i-1] > w[i] && w[i] > w[i+1] && w[i+1] > w[i+2]))
    {
      return false;
    }
  if (computeLCAd(w[i],w[i+1]) > computeLCAd(w[i-1],w[i]) &&
      computeLCAd(w[i],w[i+1]) > computeLCAd(w[i+1],w[i+2])) {
    return true;
  } else {
    return false;
  }
}

static INLINE
boolean isPair(CODE *w, int src) {
  if (isRepetition(w, src)) {
    return true;
  }
  else if (isRepetition(w, src+1)) {
    return false;
  }
  else if (isRepetition(w, src+2)) {
    return true;
  }
  else if (isMinimal(w, src) || isMaximal(w, src)) {
    return true;
  }
  else if (isMinimal(w, src+1) || isMaximal(w, src+1)) {
    return false;
  }
  return true;
}

static INLINE
CODE addRule2Dictionary(DICT *dict, CODE left, CODE right)
{
  RULE *rule = dict->rule;
  CODE *first = dict->first;
  CODE *next = dict->next;
  CODE new_key = dict->numRules;
  uint hashnum = dict->hashnum;
  CODE temp;
  uint h;
  uint i;
  
  dict->numRules++;

  if (dict->numRules > dict->buffsize) {
    dict->buffsize *= DICTIONARY_SCALING_FACTOR;

    rule = (RULE*)realloc(rule, dict->buffsize*sizeof(RULE));
    if (rule == NULL) {
      puts("Memory reallocate error (rule) at addRule2Dictionary.");
      exit(1);
    }
    next = (CODE*)realloc(next, dict->buffsize*sizeof(CODE));
    if (next == NULL) {
      puts("Memory reallocate error (next) at addRule2Dictionary.");
      exit(1);
    }
    dict->rule = rule;
    dict->next = next;
  }

  if (dict->numRules > 
      (uint)(floor((double)primes[hashnum]*LOAD_FACTOR))) {
    hashnum++;
    if (primes[hashnum] == 0) {
      puts("hash function is overflow");
      exit(1);
    }
    first = (CODE*)realloc(first, primes[hashnum]*sizeof(CODE));
    if (first == NULL) {
      puts("Memory allocate error (first) at addRule2Dictionary.");
      exit(1);
    }
    for (i = 0; i < primes[hashnum]; i++) {
      first[i] = DUMMY_CODE;
    }
    for (i = CHAR_SIZE+1; i < dict->numRules; i++) {
      next[i] = DUMMY_CODE;
    }
    for (i = CHAR_SIZE+1; i < dict->numRules; i++) {
      h =((rule[i].left * (uint)rule[i].right) % primes[hashnum]);
      next[i] = DUMMY_CODE;
      temp = first[h];
      first[h] = i;
      if (temp != DUMMY_CODE) {
	next[i] = temp;
      }
    }
    dict->hashnum = hashnum;
    dict->first = first;
  }
  
  rule[new_key].left = left;
  rule[new_key].right = right;

  if (new_key > DUMMY_CODE) {
    h = ((left * (uint)right) % primes[hashnum]);
    next[new_key] = DUMMY_CODE;
    temp = first[h];
    if (temp != DUMMY_CODE) {
      first[h] = new_key;
      next[new_key] = temp;
    } else {
      first[h] = new_key;
    }
  }
  return new_key;
}

static INLINE
CODE searchRule(DICT *dict, CODE left, CODE right)
{
  RULE *rule = dict->rule;
  CODE *first = dict->first;
  CODE *next = dict->next;
  uint hashnum = dict->hashnum;
  CODE key;
  uint h;

  h = ((left * (uint)right) % primes[hashnum]);
  key = first[h];
  while (key != DUMMY_CODE) {
    if (rule[key].left == left && rule[key].right == right) {
      return key;
    } else {
      key = next[key];
    }
  }
  return DUMMY_CODE;
}

static
DICT *createDictionary()
{
  uint i;
  DICT *dict;
  uint dictsize;
  uint hashnum;

  dict = (DICT*)malloc(sizeof(DICT));
  dictsize = INIT_DICTIONARY_SIZE;
  hashnum = INIT_HASH_NUM;
  dict->buffsize = dictsize;
  dict->hashnum = hashnum;

  dict->rule = (RULE*)malloc(dictsize*sizeof(RULE));
  if (dict->rule == NULL) {
    puts("Memory allocate error (rule) at InitDictionary.");
    exit(1);
  }

  dict->first = (CODE*)malloc(primes[hashnum]*sizeof(CODE));
  if (dict->first == NULL) {
    puts("Memory allocate error (first) at InitDictionary.");
    exit(1);
  }
  for (i = 0; i < primes[hashnum]; i++) {
    dict->first[i] = DUMMY_CODE;
  }

  dict->next = (CODE*)malloc(dictsize*sizeof(CODE));
  if (dict->next == NULL) {
    puts("Memory allocate error (next) at InitDictionary.");
    exit(1);
  }
  for (i = 0; i < dictsize; i++) {
    dict->next[i] = DUMMY_CODE;
  }

  for (i = 0; i < CHAR_SIZE; i++) {
    addRule2Dictionary(dict, i, DUMMY_CODE);
  }
  addRule2Dictionary(dict, DUMMY_CODE, DUMMY_CODE);

  return dict;
}

static
CODE *preReading(FILE *input, uint size_w)
{
  CODE c;
  CODE *w;
  uint i;

  w = (CODE *)malloc(size_w*sizeof(CODE)+2);
  if (w == NULL) {
    puts("Memory allocate error at preReading.");
    exit(1);
  }
  w++;
  w[-1] = DUMMY_CODE;

  i = 0;
  while((c=getc(input)) != EOF) {
    w[i] = c;
    i++;
  }
  w[i] = DUMMY_CODE;
  printf("Size of w at the pre_reading: %d\n", size_w);

  return w;
}

void OutputGeneratedCFG(DICT *dict, FILE *output)
{
  uint txtlen = dict->txtlen;
  uint numRules = dict->numRules;
  RULE *rule = dict->rule;

  printf("Total production rules = %d\n", numRules);
  fwrite(&txtlen, sizeof(uint), 1, output);
  fwrite(&numRules, sizeof(uint), 1, output);
  fwrite(rule+CHAR_SIZE+1, sizeof(RULE), numRules-(CHAR_SIZE+1), output);
}

void DestructDict(DICT *dict)
{
  free(dict->rule);
  free(dict->first);
  free(dict->next);
  free(dict);
}

DICT *GrammarTrans_LCA(FILE *input)
{
  uint size_w = 0;
  int src, dst;
  CODE new;
  CODE *w;
  DICT *dict;
  uint numloop = 0;
  
  fseek(input,0,SEEK_END);
  size_w = ftell(input);
  rewind(input);

  // preprocessing
  w = preReading(input, size_w);
  dict = createDictionary();
  dict->txtlen = size_w;

  while (1) {
    numloop++;
    printf("\r[ %2d ] : |w| = [ %12d ] : |D| = [ %10d ]", numloop, size_w, dict->numRules);
    fflush(stdout);
    for (src = dst = 0; src < size_w - 2; dst++) {
      if (isPair(w, src) == true) { // Is w[i,i+1] a replaced pair?
        // replace w[i, i+1] to "new".
        if ((new = searchRule(dict, w[src], w[src+1])) == DUMMY_CODE) {
          new = addRule2Dictionary(dict, w[src], w[src+1]);
        }
        w[dst] = new;
        src += 2;
      } else {
        // replace w[i+1, i+2] to "new".
        if ((new = searchRule(dict, w[src+1], w[src+2])) == DUMMY_CODE) {
          new = addRule2Dictionary(dict, w[src+1], w[src+2]);
        }
        w[dst] = w[src];
        dst++;
        w[dst] = new;
        src += 3;
      }
    }
    if (src == (size_w - 2)) {
      if ((new = searchRule(dict, w[src], w[src+1])) == DUMMY_CODE) {
        new = addRule2Dictionary(dict, w[src], w[src+1]);
      }
      w[dst] = new;
      dst++;
    } else if (src == (size_w - 1)) {
      w[dst] = w[src];
      dst++;
    }
    size_w = dst;
    if (size_w == 1) {
      break;
    }
  }
  printf("\n");
  w--;
  free(w);
  return dict;
}

