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

#ifndef TXTCFGONLINE_H
#define TXTCFGONLINE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lcacommon.h"

typedef struct Dictionary {
  uint txtlen;
  RULE *rule;
  uint numRules;
  CODE *first;
  CODE *next;
  uint buffsize;
  uint hashnum;
} DICT;


// function prototype declarations
#ifdef __cplusplus
extern "C" {
#endif
DICT *GrammarTrans_LCA(FILE *input);
void OutputGeneratedCFG(DICT *dict, FILE *output);
void DestructDict(DICT *dict);
#ifdef __cplusplus
}
#endif


#endif
