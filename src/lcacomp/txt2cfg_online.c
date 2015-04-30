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

#include "txt2cfg_online.h"

//#define DEBUG
#define INLINE __inline

#define BUFFER_SIZE (32*1024)   // size of input buffer
#define B 8   // maximum size of queue
#define M(X) ((X) % B)
#define hash_val(P_NUM, A, B) (((A)<<16|(B))%primes[(P_NUM)])
//#define hash_val(P_NUM, A, B) (((A)*(B))%primes[(P_NUM)])

typedef struct Queue {
  CODE w[B];
  uint spos;
  uint epos;
  uint num;
  struct Queue *next;
} QU;

//Inner function declaration
static QU   *createQueue       ();
static void destructQueues     (QU *q);
static void enQueue            (QU *q, CODE c);
static CODE deQueue            (QU *q);
static CODE refQueue           (QU *q, uint i);
static boolean isRepetition       (QU *q, uint i);
static boolean isMinimal          (QU *q, uint i);
static uint computeLCAd        (CODE i, CODE j);
static boolean isMaximal          (QU *q, uint i);
static boolean isPair             (QU *q);
static CODE addRule2Dictionary (DICT *dict, CODE left, CODE right);
static CODE searchRule         (DICT *dict, CODE left, CODE right);
static CODE reverseAccess      (DICT *dict, CODE left, CODE right);
static DICT *createDictionary  ();
static void grammarTrans_rec   (DICT *dict, QU *q, CODE c);

static
QU *createQueue() {
  uint i;
  QU *q = (QU*)malloc(sizeof(QU));

  for (i=0; i < B; i++) {
    q->w[i] = DUMMY_CODE;
  }
  q->spos = 0;
  q->epos = 0;
  q->num = 1;
  q->next = NULL;

  return q;
}

static INLINE
void enQueue(QU *q, CODE c)
{
  q->epos = M(q->epos+1);
  q->w[q->epos] = c;
  q->num++;
}

static INLINE
CODE deQueue(QU *q)
{
  CODE x = q->w[q->spos];
  q->spos = M(q->spos+1);
  q->num--;
  return x;
}

static INLINE
CODE refQueue(QU *q, uint i)
{
  return q->w[M(q->spos+i)];
}

static
void destructQueues(QU *q)
{
  if (q == NULL) {
    return;
  }
  destructQueues(q->next);
  free(q);
}

static INLINE
boolean isRepetition(QU *q, uint i)
{
  CODE w1 = refQueue(q, i);
  CODE w2 = refQueue(q, i+1);

  if (w1 == w2) {
    return true;
  }
  return false;
}

static INLINE
boolean isMinimal(QU *q, uint i)
{
  CODE w0 = refQueue(q, i-1);
  CODE w1 = refQueue(q, i);
  CODE w2 = refQueue(q, i+1);

  if ((w0 > w1) && (w1 < w2)) {
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
boolean isMaximal(QU *q, uint i)
{
  CODE w0 = refQueue(q, i-1);
  CODE w1 = refQueue(q, i);
  CODE w2 = refQueue(q, i+1);
  CODE w3 = refQueue(q, i+2);

  if (!(w0 < w1 && w1 < w2 && w2 < w3) && 
      !(w0 > w1 && w1 > w2 && w2 > w3))
    {
      return false;
    }
  
  if (computeLCAd(w1,w2) > computeLCAd(w0,w1) &&
      computeLCAd(w1,w2) > computeLCAd(w2,w3)) 
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

static INLINE
boolean isPair(QU *q) {
  if (isRepetition(q, 1)) {
    return true;
  }
  else if (isRepetition(q, 2)) {
    return false;
  }
  else if (isRepetition(q, 3)) {
    return true;
  }
  else if (isMinimal(q, 1) || isMaximal(q, 1)) {
    return true;
  }
  else if (isMinimal(q, 2) || isMaximal(q, 2)) {
    return false;
  }
  return true;
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

  h = hash_val(hashnum, left, right);
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

static INLINE
CODE addRule2Dictionary(DICT *dict, CODE left, CODE right)
{
  RULE *rule = dict->rule;
  CODE *first = dict->first;
  CODE *next = dict->next;
  CODE new_key = dict->numRules;
  uint hashnum = dict->hashnum;
  CODE temp;
  uint h, i;
  
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

  if (dict->numRules > (uint)(primes[hashnum]*LOAD_FACTOR)) {
    hashnum++;
    if (primes[hashnum] == 0) {
      puts("size of hash table is overflow.");
      exit(1);
    }
    first = (CODE*)realloc(first, primes[hashnum]*sizeof(CODE));
    if (first == NULL) {
      puts("Memory reallocate error (first) at addRule2Dictionary.");
      exit(1);
    }
    for (i = 0; i < primes[hashnum]; i++) {
      first[i] = (uint)DUMMY_CODE;
    }
    for (i = CHAR_SIZE+1; i < dict->numRules; i++) {
      next[i] = DUMMY_CODE;
    }
    for (i = dict->numRules-1; i > CHAR_SIZE; i--) {
      h = hash_val(hashnum, rule[i].left, rule[i].right);
      next[i] = DUMMY_CODE;
      temp = first[h];
      if (temp != DUMMY_CODE) {
	first[h] = i;
	next[i] = temp;
      } else {
	first[h] = i;
      }
    }
    dict->hashnum = hashnum;
    dict->first = first;
  }
  
  rule[new_key].left = left;
  rule[new_key].right = right;

  if (new_key > DUMMY_CODE) {
    h = hash_val(hashnum, left, right);
    next[new_key] = DUMMY_CODE;
    temp = first[h];
    first[h] = new_key;
    if (temp != DUMMY_CODE) {
      next[new_key] = temp;
    }
  }
  return new_key;
}

static INLINE
CODE reverseAccess(DICT *dict, CODE left, CODE right)
{
  CODE R;
  if ((R = searchRule(dict, left, right)) == DUMMY_CODE) {
    R = addRule2Dictionary(dict, left, right);
  }
  return R;
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

static INLINE
void grammarTrans_rec(DICT *dict, QU *p, CODE c)
{
  QU *q;
  CODE v, x1, x2, x3;

  if (p->next == NULL) {
    q = createQueue();
    p->next = q;
  }
  else {
    q = p->next;
  }

  enQueue(q, c);
  if (q->num == B) {
    if (isPair(q) == true) {
      deQueue(q);
      x1 = deQueue(q);
      x2 = refQueue(q, 0);
      v = reverseAccess(dict, x1, x2);
      grammarTrans_rec(dict, q, v);
    }
    else {
      deQueue(q);
      x1 = deQueue(q);
      grammarTrans_rec(dict, q, x1);
      x2 = deQueue(q);
      x3 = refQueue(q, 0);
      v = reverseAccess(dict, x2, x3);
      grammarTrans_rec(dict, q, v);
    }
  }
}

DICT *GrammarTrans_LCA(FILE *input)
{
  uchar *w, *w_top;
  uint lg;
  QU *dummy_que, *que;
  CODE v, x1, x2;
  DICT *dict;
  long cnt = 0;

  dummy_que = createQueue();
  que = dummy_que;
  dict = createDictionary();

  printf("Grammar Transforming ...\n");

  w_top = (uchar*)malloc(sizeof(uchar)*BUFFER_SIZE);
  while ((lg = fread(w_top, sizeof(uchar), BUFFER_SIZE, input)) > 0) {
    cnt += lg;
    w = w_top;
    do { 
      grammarTrans_rec(dict, dummy_que, *w++);
    } while (--lg > 0);
    printf("\r[ %12ld ] bytes -> [ %10d ] rules.", cnt, dict->numRules);
    fflush(stdout);
  }
  dict->txtlen = cnt;

  que = dummy_que->next;
  while (que->next != NULL || que->num > 2) {
    deQueue(que);
    while (que->num > 1) {
      x1 = deQueue(que);
      x2 = deQueue(que);
      v  = reverseAccess(dict, x1, x2);
      grammarTrans_rec(dict, que, v);
    }
    if (que->num == 1) {
      x1 = deQueue(que);
      grammarTrans_rec(dict, que, x1);
    }
    que = que->next;
  }

  printf("\r[ %12ld ] bytes -> [ %10d ] rules.\n", cnt, dict->numRules);
  free(w_top);
  destructQueues(dummy_que);
  return dict;
}

void OutputGeneratedCFG(DICT *dict, FILE *output)
{
  uint txtlen = dict->txtlen;
  uint numRules = dict->numRules;

  fwrite(&txtlen, sizeof(uint), 1, output);
  fwrite(&numRules, sizeof(uint), 1, output);
  fwrite(dict->rule+CHAR_SIZE+1, sizeof(RULE), numRules-(CHAR_SIZE+1), output);
}

void DestructDict(DICT *dict)
{
  free(dict->rule);
  free(dict->first);
  free(dict->next);
  free(dict);
}
