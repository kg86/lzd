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

// Restore CFG_file to the TXT_file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lcacommon.h"

static
void restoreString(RULE *rule, CODE code, FILE *output)
{
  CODE left, right;

  left  = rule[code].left;
  right = rule[code].right;
  if (code == left && right == DUMMY_CODE) {
    fputc(code, output);
  } else {
    restoreString(rule, left, output);
    restoreString(rule, right, output);
  }
}

int main(int argc, char *argv[])
{
  FILE *input, *output;
  CODE start;
  uint num_rules;
  uint txtlen;
  uint i;
  RULE *rule;

  if (argc != 3) {
    printf("usage: %s target_cfg_file output_txt_file\n", argv[0]);
    exit(1);
  }
  
  input  = fopen(argv[1], "rb");
  output = fopen(argv[2], "w");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }

  fread(&txtlen, sizeof(uint), 1, input);
  fread(&num_rules, sizeof(uint), 1, input);
  start = num_rules - 1;

  rule = (RULE*)malloc(sizeof(RULE)*num_rules);
  if (rule == NULL) {
    puts("Memory allocate error at InitRuleionary.");
    exit(1);
  }

  for (i = 0; i <= CHAR_SIZE; i++) {
    rule[i].left = i;
    rule[i].right = DUMMY_CODE;
  }

  fread(rule+CHAR_SIZE+1, sizeof(RULE), num_rules-(CHAR_SIZE+1), input);
  printf("Expanding CFG...");
  fflush(stdout);
  restoreString(rule, start, output);
  printf("done!!\n");

  fclose(input);
  fclose(output);
  free(rule);
  return 0;
}
