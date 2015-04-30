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

#ifndef LCACOMMON_H
#define LCACOMMON_H

#ifndef LOG2
# define LOG2(X) (log((double)(X))/log((double)2))
#endif
#define CHAR_SIZE 256
#define DUMMY_CODE (CODE)256
#define INIT_DICTIONARY_SIZE (256*1024)
#define DICTIONARY_SCALING_FACTOR (1.5)
#define INIT_HASH_NUM 15
#define LOAD_FACTOR (1.0)

#ifndef true
# define false 0
# define true  1
#endif

#define OP 1
#define CP 0

typedef unsigned int CODE;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned char boolean;

typedef struct Rule {
  CODE left;
  CODE right;
  //CODE next;
} RULE;

static const uint primes[] = {
  /* 0*/  8 + 3,
  /* 1*/  16 + 3,
  /* 2*/  32 + 5,
  /* 3*/  64 + 3,
  /* 4*/  128 + 3,
  /* 5*/  256 + 27,
  /* 6*/  512 + 9,
  /* 7*/  1024 + 9,
  /* 8*/  2048 + 5,
  /* 9*/  4096 + 3,
  /*10*/  8192 + 27,
  /*11*/  16384 + 43,
  /*12*/  32768 + 3,
  /*13*/  65536 + 45,
  /*14*/  131072 + 29,
  /*15*/  262144 + 3,
  /*16*/  524288 + 21,
  /*17*/  1048576 + 7,
  /*18*/  2097152 + 17,
  /*19*/  4194304 + 15,
  /*20*/  8388608 + 9,
  /*21*/  16777216 + 43,
  /*22*/  33554432 + 35,
  /*23*/  67108864 + 15,
  /*24*/  134217728 + 29,
  /*25*/  268435456 + 3,
  /*26*/  536870912 + 11,
  /*27*/  1073741824 + 85,
	  0
};

#endif /* LCACOMMON_H */
