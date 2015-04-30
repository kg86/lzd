#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <getopt.h>
#include <cstdlib>
#include <cassert>

#include "common.hpp"
#include "common_utils.hpp"
#include "string_tree.hpp"
#include "string_tree_pre.hpp"
#include "string_tree_occ.hpp"
#include "string_tree_clean.hpp"

#include "lzFF.hpp"
#include "lz78.hpp"
#include "slp2enc.hpp"

void usage(char * argv []){
    std::cout << "Usage  : " << argv[0] << " [options]" << std::endl
	      << "Options: " << std::endl
	      << "  -f FileName         : input file" << std::endl
	      << "  -o FileName         : output file" << std::endl
	      << "  -c                  : check whether decompressed string equals the input" << std::endl
	      << "  -d NUM              : set the debug level" << std::endl
	      << "  -l maxSize          : set max code size" << std::endl
              << "  -a lz78       : LZ78" << std::endl
              << "  -a lzd        : LZD" << std::endl
              << "  -a vfpre      : LZD VF (Prefix Base)" << std::endl
              << "  -a vfcount    : LZD VF (Count Base)" << std::endl
              << "  -a vfclean    : LZD VF (Reset Base)" << std::endl
              << "  -a vfpre_no_stream   : LZD VF (Prefix Base)" << std::endl
              << "  -a vfcount_no_stream : LZD VF (Count Base)" << std::endl;

}

namespace algo{
  std::string LZ78       = "lz78";
  std::string LZD        = "lzd";
  std::string LZVF_PRE   = "vfpre";
  std::string LZVF_COUNT = "vfcount";
  std::string LZVF_CLEAN = "vfclean";
  std::string LZVF_PRE_NO_STREAM   = "vfpre_no_stream";
  std::string LZVF_COUNT_NO_STREAM = "vfcount_no_stream";
};


int main(int argc, char * argv[]){
  int ch;
  std::string s, r, inFile;
  std::string algoname = "";
  // bool SLP = false;
  unsigned int codeSize = 0;
  std::string out_fname;
  bool help = false;
  while ((ch = getopt(argc, argv, "f:s:a:d:pl:o:h")) != -1) {
    switch (ch) {
    case 'f':
      inFile = optarg;
      break;
    case 'a':
      algoname = optarg;
      break;
    case 'd':
      UTIL::DEBUG_LEVEL = atoi(optarg);
      break;
    case 'l':
      codeSize = atoi(optarg);
      break;
    case 'o':
      out_fname = optarg;
      break;
    case 'h':
      help = true;
    }
  }
  if (help ||!(
        algoname == algo::LZD ||
        algoname == algo::LZ78 ||
        algoname == algo::LZVF_PRE || algoname == algo::LZVF_COUNT ||
        algoname == algo::LZVF_CLEAN ||
        algoname == algo::LZVF_PRE_NO_STREAM || algoname == algo::LZVF_COUNT_NO_STREAM
        )){
    codeSize = 0;
    usage(argv);
    exit(1);
  }

  // std::cerr << "comp: out_fname=[" << out_fname << "]" << std::endl;
  if (algoname == algo::LZD){
    UTIL::stringFromFile(inFile, s);
    LZFF::LZD ff;
    std::cout << (LZFF::ff_compress(s, ff)) << std::endl;
    std::cout << "convert to SLPs" << std::endl;
    std::vector<std::pair<unsigned int, unsigned int> > vars;
    std::string outSLP;
    LZFF::seq2vars(ff.seq, vars);
    std::cout << "vars.size()=" << vars.size() << std::endl;
    slp2enc(vars, (unsigned int) s.size(), out_fname);
  } else if (algoname == algo::LZ78) {
    std::vector<std::pair<unsigned int, unsigned int> > vars;
    LZ78::LZ78 lz78;
    UTIL::stringFromFile(inFile, s);
    LZ78::compress(s, lz78);
    LZ78::seq2vars(lz78.seq, vars);
    slp2enc(vars, (unsigned int) s.size(), out_fname);
  }else if (algoname == algo::LZVF_PRE) {
    STreePre::STreePreCompress(inFile, out_fname, codeSize);
  }else if (algoname == algo::LZVF_COUNT) {
    STreeOcc::STreeOccCompress(inFile, out_fname, codeSize);
  }else if (algoname == algo::LZVF_CLEAN) {
    STreeClean::STreeCleanCompress(inFile, out_fname, codeSize);
  }else if (algoname == algo::LZVF_PRE_NO_STREAM) {
    assert(codeSize > 0);
    UTIL::stringFromFile(inFile, s);
    LZFF::LZD ff;
    std::cout << LZFF::ffpre_compress(s, codeSize, ff) << std::endl;
    ff.out(out_fname);
  }else if (algoname == algo::LZVF_COUNT_NO_STREAM) {
    assert(codeSize > 0);
    UTIL::stringFromFile(inFile, s);
    LZFF::LZD ff;
    std::cout << LZFF::ffocc_compress(s, codeSize, ff) << std::endl;
    ff.out(out_fname);
  }

  return 0;
}
