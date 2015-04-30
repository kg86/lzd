#pragma once

#include <vector>

#include "lcacomp/cfg2enc.h"
#include "lcacomp/enc2txt.h"

void slp2enc(std::vector<std::pair<unsigned int, unsigned int> > & vars, unsigned int decompressedSize, const std::string & outputFilename);
void enc2slp(FILE * input, std::vector<std::pair<int, int> > & vars);

int encSLP_decompress(std::string & in_fname, std::string & out_fname);
