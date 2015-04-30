#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "txt2cfg_online.h"
#include "cfg2enc.h"
#include "enc2txt.h"

int decompression(char *input_filename, char *output_filename)
{
  FILE *input, *output;

  input = fopen(input_filename, "rb");
  output = fopen(output_filename, "w");
  if (input == NULL || output == NULL) {
    printf("File open error.\n");
    return 0;
  }
  DecodeCFG(input, output);
  fclose(input); fclose(output);
  return 1;
}

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

int compression(char *input_filename, char *output_filename)
{
  FILE *input, *output;
  DICT *dict;
  EDICT *edict;

  input  = fopen(input_filename, "r");
  output = fopen(output_filename, "wb");
  if (input == NULL || output == NULL) {
    puts("File open error at the beginning.");
    return 0;
  }

  dict = GrammarTrans_LCA(input);
  edict = convertDict(dict);

  EncodeCFG(edict, output);
  DestructEDict(edict);
  fclose(input);
  fclose(output);
  return 1;
}

void printUsage(char *program_name)
{
  printf("\n");
  printf("---Usage-------------------------------------------------------------\n");
  printf("%s (c|d) <input_file> <output_file>\n", program_name);
  printf("c = Compresses <input_file> with LCA-online and creates <output_file>.\n");
  printf("d = Decompresses <input_file> and creates <output_file>.\n");
  printf("---------------------------------------------------------------------\n");
  printf("\n");
}

int main(int argc, char *argv[])
{

  if (argc != 4) {
    printUsage(argv[0]);
    exit(0);
  }

  if (!strcmp(argv[1], "c")) {
    compression(argv[2], argv[3]);
  }
  else if (!strcmp(argv[1], "d")) {
    decompression(argv[2], argv[3]);
  }
  else {
    printf("Unexpected argment!\n");
    printUsage(argv[0]);
    exit(1);
  }

  exit(0);
}

