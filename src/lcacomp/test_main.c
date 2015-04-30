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

#ifdef _TXT2CFG_ONLINE
#include"txt2cfg_online.h"

int main(int argc, char *argv[])
{
  char *target_filename;
  char *output_filename;
  FILE *input, *output;
  DICT *dict;

  if (argc != 3) {
    printf("usage: %s target_text_file output_cfg_file\n", argv[0]);
    exit(1);
  }
  target_filename = argv[1];
  output_filename = argv[2];
  
  input  = fopen(target_filename, "r");
  output = fopen(output_filename, "wb");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }

  dict = GrammarTrans_LCA(input);
  OutputGeneratedCFG(dict, output);

  fclose(input);
  fclose(output);
  DestructDict(dict);
  exit(0);
}
#endif


#ifdef _TXT2CFG_OFFLINE
#include "txt2cfg_offline.h"

int main(int argc, char *argv[])
{
  char *target_filename;
  char *output_filename;
  FILE *input, *output;
  DICT *dict;

  // analize options
    if (argc != 3) {
      printf("usage: %s target_text_file output_cfg_file\n", argv[0]);
      exit(1);
  }

  target_filename = argv[1];
  output_filename = argv[2];
  
  // Run the algorithm procedure
  input  = fopen(target_filename, "r");
  output = fopen(output_filename, "wb");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }

  dict = GrammarTrans_LCA(input);
  OutputGeneratedCFG(dict, output);
  DestructDict(dict);

  fclose(input);
  fclose(output);
  
  exit(0);
}
#endif


#ifdef _CFG2ENC
#include "cfg2enc.h"

int main(int argc, char *argv[])
{
  FILE *input, *output;
  EDICT *dict;

  if (argc != 3) {
    printf("usage: %s target_cfg_file output_enc_file\n", argv[0]);
    exit(1);
  }
  input = fopen(argv[1], "rb");
  output = fopen(argv[2], "wb");
  if (input == NULL || output == NULL) {
    printf("File open error.\n");
    exit(1);
  }
  dict = ReadCFG(input);
  EncodeCFG(dict, output);
  DestructEDict(dict);
  fclose(input); fclose(output);
  exit(0);
}
#endif

#ifdef _ENC2TXT
#include "enc2txt.h"
int main(int argc, char *argv[])
{
  FILE *input, *output;

  if (argc != 3) {
    printf("usage: %s target_enc_file output_txt_file\n", argv[0]);
    puts("argument error.");
    exit(1);
  }
  input = fopen(argv[1], "rb");
  output = fopen(argv[2], "w");
  if (input == NULL || output == NULL) {
    printf("File open error.\n");
    exit(1);
  }
  DecodeCFG(input, output);
  fclose(input); fclose(output);
  exit(0);
}
#endif


#ifdef _TXT2ENC_ONLINE
#include "txt2cfg_online.h"
#include "cfg2enc.h"

EDICT *convertDict(DICT *dict)
{
  EDICT *edict = (EDICT*)malloc(sizeof(EDICT));
  uint i;
  edict->txtlen = dict->txtlen;
  edict->start = dict->numRules-1;
  edict->rule  = dict->rule;
  edict->numRules = dict->numRules;
  edict->tcode = dict->next;
  edict->newcode = CHAR_SIZE;

  for (i = 0; i <= CHAR_SIZE; i++) {
    edict->tcode[i] = i;
  }
  for (i = CHAR_SIZE+1; i < dict->numRules; i++) {
    edict->tcode[i] = DUMMY_CODE;
  }

  free(dict->first);
  free(dict);
  return edict;
}

int main(int argc, char *argv[])
{
  char *target_filename;
  char *output_filename;
  FILE *input, *output;
  DICT *dict;
  EDICT *edict;

  if (argc != 3) {
    printf("usage: %s target_text_file output_enc_file\n", argv[0]);
    exit(1);
  }
  target_filename = argv[1];
  output_filename = argv[2];
  
  input  = fopen(target_filename, "r");
  output = fopen(output_filename, "wb");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }

  dict = GrammarTrans_LCA(input);
  edict = convertDict(dict);

  EncodeCFG(edict, output);
  DestructEDict(edict);
  fclose(input);
  fclose(output);
  exit(0);
}
#endif


#ifdef _TXT2ENC_OFFLINE
#include "txt2cfg_offline.h"
#include "cfg2enc.h"

EDICT *convertDict(DICT *dict)
{
  EDICT *edict = (EDICT*)malloc(sizeof(EDICT));
  uint i;
  edict->txtlen = dict->txtlen;
  edict->start = dict->numRules-1;
  edict->rule  = dict->rule;
  edict->numRules = dict->numRules;
  edict->tcode = dict->next;
  edict->newcode = CHAR_SIZE;

  for (i = 0; i <= CHAR_SIZE; i++) {
    edict->tcode[i] = i;
  }
  for (i = CHAR_SIZE+1; i < dict->numRules; i++) {
    edict->tcode[i] = DUMMY_CODE;
  }

  free(dict->first);
  free(dict);
  return edict;
}

int main(int argc, char *argv[])
{
  char *target_filename;
  char *output_filename;
  FILE *input, *output;
  DICT *dict;
  EDICT *edict;

  if (argc != 3) {
    printf("usage: %s target_text_file output_enc_file\n", argv[0]);
    exit(1);
  }
  target_filename = argv[1];
  output_filename = argv[2];
  
  input  = fopen(target_filename, "r");
  output = fopen(output_filename, "wb");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    exit(1);
  }

  dict = GrammarTrans_LCA(input);
  edict = convertDict(dict);
  EncodeCFG(edict, output);
  DestructEDict(edict);
  fclose(input);
  fclose(output);
  exit(0);
}
#endif
