// -*- coding: utf-8 -*-
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <cassert>
#include <sstream>
#include <fstream>

#include "common_utils.hpp"
#include "common.hpp"

#include "lzFF.hpp"
#include "string_tree_ns.hpp"
#include "string_tree_limited_ns.hpp"
#include "string_tree_pre_ns.hpp"
#include "string_tree_occ_ns.hpp"
#include "slp2enc.hpp"

namespace LZFF{
  LZD::LZD(unsigned int fileSize, unsigned int codeSize, std::string & out_fname) : seq(), fileSize(fileSize), codeSize(codeSize){
    os.open(out_fname.c_str());
  }
  LZD::LZD(const std::string & fname){
    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary);
    unsigned int seqSize;
    unsigned int last_right_factor;
    f.read(reinterpret_cast<char *> (&this->fileSize), sizeof(this->fileSize));
    f.read(reinterpret_cast<char *> (&seqSize), sizeof(seqSize));
    f.read(reinterpret_cast<char *> (&this->codeSize), sizeof(this->codeSize));
    f.read(reinterpret_cast<char *> (&last_right_factor), sizeof(last_right_factor));
    seq.reserve(seqSize + sizeof(long long));

    const unsigned int codePairSize = 2*codeSize;
    unsigned long long in_bit; // buffer
    const unsigned int NUM_BITS = sizeof(in_bit) * 8; // number of bit

    unsigned long long bit = 0;
    unsigned int mask = (1 << codeSize)-1;
    if(!f.read(reinterpret_cast<char *> (&in_bit), sizeof(in_bit))){
      std::cerr << "error" << std::endl;
    }
    unsigned int remain_bitsize = NUM_BITS;
    while(seq.size() < seqSize){
      while (remain_bitsize >= codePairSize){
        assert(remain_bitsize <= NUM_BITS);
        std::pair<int, int> v;
        v.first = mask & (in_bit >> (remain_bitsize-codeSize));
        v.second = mask&(in_bit >> (remain_bitsize-codePairSize));
        seq.push_back(v);
        remain_bitsize -= codePairSize;
      }
      assert(remain_bitsize <= NUM_BITS);
      bit = ((1<<codePairSize)-1) & (in_bit << (codePairSize-remain_bitsize));
      remain_bitsize = NUM_BITS - (codePairSize - remain_bitsize);

      if(!f.read(reinterpret_cast<char *> (&in_bit), sizeof(in_bit))){
        break;
      }
      bit |= (in_bit >> remain_bitsize);
      std::pair<unsigned int, unsigned int> v;
      v.first = mask&(bit >> codeSize);
      v.second = mask&bit;
      seq.push_back(v);
    }

    // removes the unused last element when reads extra amount
    while(seqSize < seq.size()) seq.pop_back();
    seq.back().second = last_right_factor;
  }

  void LZD::out(const std::string & out_fname){
    std::ofstream os;
    os.open(out_fname.c_str());
    unsigned int i;
    unsigned int seqSize = (unsigned int) this->seq.size();
    unsigned int last_right_factor = this->seq.back().second;
    os.write(reinterpret_cast<const char *> (&this->fileSize), sizeof(this->fileSize));
    os.write(reinterpret_cast<const char *> (&seqSize), sizeof(seqSize));
    os.write(reinterpret_cast<const char *> (&this->codeSize), sizeof(this->codeSize));
    os.write(reinterpret_cast<const char *> (&last_right_factor), sizeof(last_right_factor));

    const unsigned long long mask = (1 << this->codeSize)-1;
    unsigned long long bit = 0; // temporary bits
    unsigned long long out_bit = 0; // temporary bits for writing
    const unsigned int codePairSize = 2*this->codeSize;
    const unsigned int out_bit_size = 8 * sizeof(out_bit);
    unsigned int out_remain = out_bit_size;
    unsigned int count = 0;
    for(i = 0; i < seq.size(); i++){
      if (this->seq[i].first > mask ||
          (this->seq[i].first != 0 && this->seq[i].second > mask)){
        std::cerr << "ERROR [" << this->seq[i].first << ", "
                  << this->seq[i].second << "] bit=" << bit
                  << " first from bit=" << ((bit >> this->codeSize) & mask)
                  << " second from bit=" << (bit & mask)
                  << " hoge=" << (bit << (out_remain-codePairSize))
                  <<std::endl;
      }
      assert(seq[i].first <= mask);
      assert(seq[i].second <= mask || seq[i].first == 0); // root
      bit = mask & seq[i].first;
      bit = bit << codeSize;
      bit |= mask & seq[i].second;

      if (out_remain >= codePairSize){
        out_bit |= bit << (out_remain-codePairSize);
        out_remain -= codePairSize;
      }else {
        // copies $out_remain bits from $bit
        out_bit |= bit >> (codePairSize-out_remain);
        os.write(reinterpret_cast<const char *> (&out_bit), sizeof(out_bit));
        out_bit = 0;
        // copies the remain/unwritten bits from $bit
        out_remain = (out_bit_size - (codePairSize - out_remain));
        out_bit = bit << out_remain;
        count++;
      }
    }
    if (out_bit > 0){
      os.write(reinterpret_cast<const char *> (&out_bit), sizeof(out_bit));
    }
  }

  unsigned int ff_compress(LZD & lz, NOT_STREAM::STree::Tree & tree){
    assert(tree.root != NULL);

    unsigned int p = 0; // current position of the input string
    unsigned int n = (unsigned int) tree.getStrSize();
    lz.seq.push_back(std::make_pair(0,0)); // for root node
    NOT_STREAM::STree::Node * firstFactor = NULL;
    while (p < n){
      NOT_STREAM::STree::Node * cur_factor = tree.findLastFNodeFrom(tree.root, p);
      assert(p + cur_factor->
             depth <= n);
      if (cur_factor == tree.root){
        lz.seq.push_back(std::make_pair(tree.root->fid, (unsigned char) tree.getCharAt(p)));
        cur_factor = tree.insertCharNode(tree.getCharAt(p));
        assert(cur_factor->depth == 1);
        assert(cur_factor->isFactor());
      }
      if (firstFactor == NULL){
        firstFactor = cur_factor;
      }else {
        lz.seq.push_back(std::make_pair(firstFactor->fid, cur_factor->fid));
        const NOT_STREAM::STree::Node * new_node = tree.insertFactorNode(firstFactor, p, cur_factor->depth);

        if (UTIL::DEBUG_LEVEL > 2){

          const unsigned int prev_p = p - firstFactor->depth;
          std::string ls, rs;
          ls = "";
          rs = "";
          tree.toString(firstFactor, ls);
          tree.toString(cur_factor, rs);
          std::cerr << "[" << new_node->fid << "]=" 
                    << "[" << firstFactor->fid << "]"
                    << "[" << cur_factor->fid << "]="
                    << "[" << ls << "]"
                    << "[" << rs << "]"
                    << std::endl;
          if (ls != tree.substr(prev_p, firstFactor->depth)){
            std::cerr << "prev_p=" << prev_p
                      << " depth=" << firstFactor->depth << std::endl;
            // std::cerr.flush();
            std::cerr << "origin[" << tree.substr(prev_p, firstFactor->depth) <<  "] "
                      << "firstfactor[" << ls << "]" << std::endl;
          }
          if (rs != tree.substr(p, cur_factor->depth)){
            std::cerr << "origin[" << tree.substr(p, cur_factor->depth) <<  "] "
                      << "secondfactor[" << rs << "]" << std::endl;
          }
        }
        firstFactor = NULL;
      }
      p += cur_factor->depth;
    }
    if (firstFactor != NULL){
      lz.seq.push_back(std::make_pair(firstFactor->fid, tree.root->fid));
    }

    return (unsigned int) lz.seq.size();
  }

  unsigned int ff_compress(std::string & str, LZD & lz){
    NOT_STREAM::STree::Tree tree(str);
    return ff_compress(lz, tree);
  }

  void seq2vars(const std::vector<std::pair<unsigned int, unsigned int> > & in_seq,
                std::vector<std::pair<unsigned int, unsigned int> > & vars){
    std::vector<unsigned int> * seq = new std::vector<unsigned int>();
    std::map<std::pair<unsigned int, unsigned int>, unsigned int>::iterator itr;
    std::vector<unsigned int> s2v(in_seq.size());
    assert(s2v.size() == in_seq.size());
    vars.clear();
    unsigned int cur = CHAR_SIZE;
    unsigned int seqsize = 0;
    vars.reserve(in_seq.size() * 2);
    seq->reserve(in_seq.size());
    assert(seq->size() == 0);

    // stores dummy values for SLPs
    for(size_t i = 0; i < CHAR_SIZE; i++){
      vars.push_back(std::make_pair(-1, -1));
    }

    // renumbers variable ids for SLPs
    for(size_t i = 1; i < in_seq.size(); i++){
      assert(cur < in_seq.size() + CHAR_SIZE);
      if (in_seq[i].first == 0){
        // the factor represents a character
        s2v[i] = in_seq[i].second;
        assert(0 <= s2v[i] && s2v[i] <= 255);
      }else if ((i+1) == in_seq.size() && in_seq.back().second == 0){
        // ignores the right of the last factor
        seq->push_back(s2v[in_seq.back().first]);
        seqsize++;
      } else {
        // a pair of factors
        s2v[i] = cur;
        vars.push_back(std::make_pair(s2v[in_seq[i].first], s2v[in_seq[i].second]));
        seq->push_back(cur);
        cur++;
        seqsize++;
      }
    }
    assert(cur == vars.size());
    assert(seqsize == seq->size());

    // computes an SLP by concatenating adjacent factors
    // until the length of the sequence becomes 1
    while(seqsize > 1){
      unsigned int new_seqsize = 0;
      for(size_t i = 1; i < seqsize; i+=2){
        vars.push_back(std::make_pair(seq->at(i-1), seq->at(i)));
        seq->at(new_seqsize) = cur;
        cur++;
        new_seqsize++;
      }
      if (seqsize % 2 == 1){
        seq->at(new_seqsize) = seq->at(seqsize-1);
        new_seqsize++;
      }
      seqsize = new_seqsize;
    }
    delete seq;
  }

  inline NOT_STREAM::STree::Node * getLPF(LZD &ff, NOT_STREAM::STreeLimit * tree, unsigned int p){
    NOT_STREAM::STree::Node * curFactor = tree->findLastFNodeFrom(tree->root, p);
    assert(curFactor != NULL);
    assert(curFactor->isFactor());
    assert(p + curFactor->depth <= tree->getStrSize());

    if(curFactor == tree->root){
      const char new_char = tree->getCharAt(p);
      ff.seq.push_back(std::make_pair(tree->root->fid, new_char));
      curFactor = tree->insertCharNode(new_char);
      assert(curFactor != tree->root);
      assert(curFactor->isFactor());
      if (UTIL::DEBUG_LEVEL > 1){
        std::cerr << "new char was detected [" << new_char << "]:";
        curFactor->info();
      }
    }
    return curFactor;
  }

  unsigned int fflimit_compress(LZD &ff, NOT_STREAM::STreeLimit * tree){
  // unsigned int ffocc_compress(LZFF &ff, STreeOcc * tree){
    unsigned int p = 0; // current position of the input string
    size_t n = tree->getStrSize();
    NOT_STREAM::STree::Node * firstFactor = NULL;
    ff.seq.push_back(std::make_pair(0, 0)); // for root
    unsigned int next = (unsigned int) n / 10;
    while (p < n){
      if (UTIL::DEBUG_LEVEL > 0){
        if (p >= next){
          std::cerr << p << "/" << n  << " seq.size()=" << ff.seq.size()<< std::endl;
          next += (unsigned int) n / 10;
        }
        if (UTIL::DEBUG_LEVEL > 1){
          std::cerr << p << "/" << n  << " seq.size()=" << ff.seq.size()<< std::endl;
        }
      }

      NOT_STREAM::STree::Node * curFactor = getLPF(ff, tree, p);
      if (firstFactor == NULL) {
        firstFactor = curFactor;
        p += curFactor->depth;
      }else  {
        tree->insertNewNode(p, firstFactor, curFactor);
        ff.seq.push_back(std::make_pair(firstFactor->fid, curFactor->fid));

        p += curFactor->depth;
        firstFactor = NULL;
        tree->delIFfull();
      }
    }
    if (firstFactor != NULL){
      ff.seq.push_back(std::make_pair(firstFactor->fid, tree->root->fid));
    }

    if (UTIL::DEBUG_LEVEL > 0){
      std::cerr << "num_fnodes=" << tree->getNumFNodes()
                << " num_nodes=" << tree->getNumNodes() << std::endl;
      std::cerr << "ff.seq.size()=" << ff.seq.size() << std::endl;
    }
    return tree->getNumFNodes();
  }

  unsigned int ffpre_compress(const std::string & str, unsigned int codeSize, LZD & ff){
    ff.codeSize = codeSize;
    ff.fileSize = (unsigned int) str.size();
    NOT_STREAM::STreePre tree(str, (1 << ff.codeSize));
    return fflimit_compress(ff, &tree);
  }

  unsigned int ffocc_compress(const std::string & str, unsigned int codeSize, LZD & ff){
    ff.codeSize = codeSize;
    ff.fileSize = (unsigned int) str.size();
    NOT_STREAM::STreeOcc tree(str, (1 << ff.codeSize));
    return fflimit_compress(ff, &tree);
  }

  void fflimit_decompress(const LZD & ff, std::string & str, NOT_STREAM::STreeLimit & tree){
    unsigned int p = 0;
    for(unsigned int i = 1; i < ff.seq.size(); i++){

      if (UTIL::DEBUG_LEVEL > 2){
        std::cerr << i << "/" << ff.seq.size() <<" p=" << p << std::endl;
        std::cerr << "seq: " << i << "/" << ff.seq.size() <<" p=" << p
                  << " first=" << ff.seq[i].first
                  << " second=" << ff.seq[i].second << std::endl;
      }

      if(ff.seq[i].first == 0){
        tree.insertCharNode((unsigned char)ff.seq[i].second);
        continue;
      }

      NOT_STREAM::STree::Node * left = tree.getFNode(ff.seq[i].first);
      tree.toString(left, str);

      if (ff.seq[i].second == 0) {
        // std::cerr << i << "/" << ff.seq.size()
        //           << " end" << std::endl;
        break;
      }

      // 効率的ではないので最適化したい
      NOT_STREAM::STree::Node * right = tree.getFNode(ff.seq[i].second);
      tree.toString(right, str);

      // ----------- DEBUG BEGIN -------------------
      if (UTIL::DEBUG_LEVEL > 2){
        std::string tmpl;
        std::string tmpr;
        tree.toString(left, tmpl);
        tree.toString(right, tmpr);
        std::cerr << "left(" << left->fid<<")=[" << tmpl << "]" << std::endl;
        std::cerr << "right" << right->fid <<"=[" << tmpr << "]" << std::endl;
        std::cerr << "left->depth=" << left->depth
                  << " left->fid_=" << left->fid
                  << " right->depth=" << right->depth
                  << " right->fid_=" << right->fid
                  << std::endl;
      }
      // ----------- DEBUG END -------------------

      NOT_STREAM::STree::Node * new_node = tree.insertNewNode(p + left->depth, left, right);
      p += new_node->depth;
      tree.delIFfull();
      if (UTIL::DEBUG_LEVEL > 2){
        tree.info();
      }
    }
  }

  void ffpre_decompress(const LZD & ff, std::string & str){
    NOT_STREAM::STreePre tree(str, 1 << ff.codeSize);
    fflimit_decompress(ff, str, tree);
  }

  void ffocc_decompress(const LZD & ff, std::string & str){
    NOT_STREAM::STreeOcc tree(str, 1 << ff.codeSize);
    fflimit_decompress(ff, str, tree);
  }

}
