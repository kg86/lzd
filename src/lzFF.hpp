// -*- coding: utf-8 -*-
#pragma once

#include <string>
#include <vector>
#include <climits>
#include <iostream>
#include <fstream>
#include <sstream>

#include "string_tree_ns.hpp"

namespace LZFF {
  using namespace NOT_STREAM;
  class FileInfo {
    public:
    unsigned long long fileSize;
    unsigned long long seqSize;
    unsigned int codeSize;
    unsigned int last_right_factor;
    FileInfo(unsigned int fileSize, unsigned int seqSize,
             unsigned int codeSize, unsigned int last_right_factor) :
      fileSize(fileSize), seqSize(seqSize),
      codeSize(codeSize), last_right_factor(last_right_factor){};
  };
  inline void writeHeader(std::ostream & os,
    FileInfo & info){
    os.write(reinterpret_cast<const char *> (&info.fileSize), sizeof(info.fileSize));
    os.write(reinterpret_cast<const char *> (&info.seqSize), sizeof(info.seqSize));
    os.write(reinterpret_cast<const char *> (&info.codeSize), sizeof(info.codeSize));
    os.write(reinterpret_cast<const char *> (&info.last_right_factor), sizeof(info.last_right_factor));
  }
  inline void readHeader(std::istream & is, FileInfo & info){
    is.read(reinterpret_cast<char *> (&info.fileSize), sizeof(info.fileSize));
    is.read(reinterpret_cast<char *> (&info.seqSize), sizeof(info.seqSize));
    is.read(reinterpret_cast<char *> (&info.codeSize), sizeof(info.codeSize));
    is.read(reinterpret_cast<char *> (&info.last_right_factor), sizeof(info.last_right_factor));
  }
  inline std::string show(FileInfo & info) {
    std::ostringstream str;
    str << "fileSize=" << info.fileSize << ", seqSize=" << info.seqSize
        << ", codeSize" << info.codeSize << ", last_right_factor" << info.last_right_factor;
    return str.str();
  }

  class LZD{
  public:
    std::vector<std::pair<unsigned int, unsigned int> > seq;
    unsigned int fileSize; // decompressed string size
    unsigned int codeSize; // encoding code size
    NOT_STREAM::STree::Node * cur;
    unsigned int len_matched;
    std::ofstream os;

    LZD() : fileSize(0), codeSize(0){};
    LZD(unsigned int fileSize, unsigned int codeSize, std::string & out_fname);

    LZD(const std::string & fname);
    void out(const std::string & out_fname);

  };

  /**
   *   computes LZD
   *
   *  @param lz
   *  @param tree
   *
   *  @return the length of factor sequence
   */
  unsigned int ff_compress(LZD & lz, NOT_STREAM::STree::Tree & tree);

  /**
   *  computes LZD
   *
   *  @param str
   *  @param lz
   *
   *  @return the length of factor sequence
   */
  unsigned int ff_compress(std::string & str, LZD & lz);

  /**
   *  computes LZD VF based on prefix strategy
   *
   *  @param str
   *  @param ff
   *
   *  @return the length of the factor sequence
   */
  unsigned int ffpre_compress(const std::string & str, unsigned int codeSize, LZD & ff);

  /**
   *  computes LZD VF based on the count strategy.
   *
   *  @param str input string
   *  @param ff  lzvf
   *
   *  @return the length of the lzvf sequence
   */
  unsigned int ffocc_compress(const std::string & str, unsigned int codeSize, LZD & ff);

  /**
   *  decompresses LZD
   *
   *  @param lz
   *  @param str
   */
  void ff_decompress(const LZD & lz, std::string & str);

  /**
   *  decompresses LZD VF based on prefix strategy
   *
   *  @param ff
   *  @param str
   */
  void ffpre_decompress(const LZD & ff, std::string & str);

  /**
   *  decompresses LZD VF based on count strategy
   *
   *  @param ff
   *  @param str
   */
  void ffocc_decompress(const LZD & ff, std::string & str);

  /**
   *  computes an SLP from a factor sequence of LZD by concatenating adjacent factors
   *
   *  @param seq  factor sequence
   *  @param vars output SLP
   */
  void seq2vars(const std::vector<std::pair<unsigned int, unsigned int> > & seq, std::vector<std::pair<unsigned int, unsigned int> > & vars);

};
