#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <getopt.h>
#include <cstdlib>
#include <cassert>

#include "common.hpp"
#include "lzFF.hpp"
#include "lz78.hpp"
#include "string_tree_pre.hpp"
#include "string_tree_occ.hpp"
#include "string_tree_clean.hpp"
#include "slp2enc.hpp"

void usage(char * argv []){
    std::cout << "Usage  : " << argv[0] << " [options]" << std::endl
	      << "Options               : " << std::endl
	      << "  -f FileName         : input file" << std::endl
	      << "  -o FileName         : output file" << std::endl
	      << "  -d NUM              : debug mode" << std::endl
              << "  -a lz78       : LZ78" << std::endl
              << "  -a lzd        : LZD" << std::endl
              << "  -a vfpre      : LZD VF (Prefix Base)" << std::endl
              << "  -a vfcount    : LZD VF (Count Base)" << std::endl
              << "  -a vfclean    : LZD VF (Reset Base)" << std::endl
              << "  -a vfpre_no_stream   : LZD VF (Prefix Base)" << std::endl
              << "  -a vfcount_no_stream : LZD VF (Count Base)" << std::endl;

}

namespace algo{
  std::string LZD = "lzd";
  std::string LZ78 = "lz78";
  std::string LZVF_PRE   = "vfpre";
  std::string LZVF_COUNT = "vfcount";
  std::string LZVF_CLEAN = "vfclean";
  std::string LZVF_PRE_NO_STREAM   = "vfpre_no_stream";
  std::string LZVF_COUNT_NO_STREAM = "vfcount_no_stream";
};

int main(int argc, char * argv[]){
  int ch;
  std::string algoname;
  std::string in_fname, out_fname;
  while ((ch = getopt(argc, argv, "f:a:d:o:h")) != -1) {
    switch (ch) {
    case 'f':
      in_fname = optarg;
      break;
    case 'a':
      algoname = optarg;
      break;
    case 'd':
      UTIL::DEBUG_LEVEL = atoi(optarg);
      break;
    case 'o':
      out_fname = optarg;
      break;
    case 'h':
      usage(argv);
      exit(1);
    }
  }

  // std::cout << "input file : " << in_fname << std::endl;
  // std::cout << "output file : " << out_fname << std::endl;
  std::ifstream ifs(in_fname.c_str(), std::ios::in | std::ios::binary);
  std::ofstream iofs(out_fname.c_str());
  if (algoname == algo::LZD ||
      algoname == algo::LZ78) {
    encSLP_decompress(in_fname, out_fname);
  } else if (algoname == algo::LZ78) {
    std::string decompressed_str;
    LZ78::LZ78 lz78(in_fname);
    LZ78::decompress(decompressed_str, lz78);
    iofs << decompressed_str;
  }else if (algoname == algo::LZVF_PRE){
    STreePre::STreePreDecompress(ifs, iofs);
  } else if (algoname == algo::LZVF_COUNT){
    STreeOcc::STreeOccDecompress(ifs, iofs);
  } else if (algoname == algo::LZVF_CLEAN){
    STreeClean::STreeCleanDecompress(ifs, iofs);
  } else if (algoname == algo::LZVF_PRE_NO_STREAM){
    LZFF::LZD ff(in_fname);
    std::string decompressed_str;
    std::cout << "seq.size() : " << ff.seq.size() << std::endl;
    LZFF::ffpre_decompress(ff, decompressed_str);
    iofs << decompressed_str;
  } else if (algoname == algo::LZVF_COUNT_NO_STREAM){
    LZFF::LZD ff(in_fname);
    std::string decompressed_str;
    std::cout << "seq.size() : " << ff.seq.size() << std::endl;
    LZFF::ffocc_decompress(ff, decompressed_str);
    iofs << decompressed_str;
  }

  return 0;
}
