#pragma once

#include <string>
#include <vector>
#include <climits>
#include <cassert>
#include <iostream>

#include "string_tree.hpp"
#include "string_tree_limited.hpp"
#include "common_utils.hpp"
#include "lzFF.hpp"

namespace STreeClean {
  class STreeClean : public STreeLimit{

  public:
    STreeClean(std::istream & is, std::ostream & os,
               unsigned int codeSize):
      STreeLimit(is, os, codeSize){};

    ~STreeClean(){};

    inline unsigned int delIFfull(){
      // deletes all nodes
      if (this->isFull()){
        for(unsigned int i = 1; i < this->max_fnode ; i++){
          STree::Node * node = this->getFNode(i);
          if (node != NULL)
            this->delNode(node);
        }
      }
      return 0;
    }
    inline STree::Node * insertCharNodeNotWrite(char c){
      STree::Node * new_node = this->addChild(this->root, true, std::string(1, c));
      if(this->stat) {
        assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
        this->stat->len_factors.push_back(1);
        this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
      }
      return new_node;
    }

    inline STree::Node * insertNewNode(STree::Node * node1, const STree::Node * node2, std::string & remain_string){
      STree::Node * new_node = this->insertFactorNode(node1, remain_string, 0, node2->depth);
      if (UTIL::DEBUG_LEVEL > 1){
        std::cerr << "INSERT NEW NODE" << std::endl;
        std::cerr << "INSERT NEW NODE " << new_node->fid << " = ("
                  << node1->fid << ")(" << node2->fid << ")"
                  <<std::endl;
      }
      if(this->stat) {
        assert(this->stat->pos_prev_used[new_node->fid] == UINT_MAX);
        this->stat->pos_prev_used[new_node->fid] = this->stat->len_factors.size()-1;
      }
      return new_node;
    }

    inline STree::Node * getLRUNode(){return NULL;};

    inline void useNode(const STree::Node * node){
      // do nothing
    }
  };

  void STreeCleanCompress(std::string & in_fname, std::string & out_fname,
                        unsigned int codeSize){
    std::ifstream ifs(in_fname.c_str());
    std::ofstream ofs(out_fname.c_str(), std::ios::binary);
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(ofs, info);
    STreeClean tree(ifs, ofs, codeSize);
    tree.remain_bitsize = tree.NUM_BITS;
    tree.compress(info);
    ifs.close();
    ofs.close();

    std::fstream fs(out_fname.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    writeHeader(fs, info);
    fs.close();
  }

  void STreeCleanCompress(std::istream & is, std::ostream & os,
                        unsigned int codeSize){
    LZFF::FileInfo info(0, 0, codeSize, 0);
    LZFF::writeHeader(os, info);
    STreeClean tree(is, os, codeSize);
    tree.remain_bitsize = tree.NUM_BITS;
    tree.compress(info);
  }

  void STreeCleanDecompress(std::istream & is, std::ostream & os){
    LZFF::FileInfo info(0, 0, 0, 0);
    LZFF::readHeader(is, info);
    STreeClean tree(is, os, info.codeSize);
    tree.remain_bitsize = 0;
    tree.decompress(info);
  }
}
